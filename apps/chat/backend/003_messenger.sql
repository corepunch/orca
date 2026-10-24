-- Run after schema.sql and 002_contacts.sql as the project administrator.
-- Deploy the authenticated finalize-chat-photo Edge Function before media upload.
begin;

alter table public.chat_rooms
  add column kind text not null default 'group' check (kind in ('direct', 'group', 'channel')),
  add column visibility text not null default 'private' check (visibility in ('private', 'public')),
  add column owner_id uuid references auth.users(id),
  add column direct_pair text unique;
-- The prototype did not record creators. Review this deterministic owner
-- assignment for any existing rooms before enabling administration in production.
update public.chat_rooms r set owner_id = (
  select m.user_id from public.chat_members m where m.room_id = r.id order by m.user_id limit 1
);
alter table public.chat_rooms alter column owner_id set not null;
alter table public.chat_rooms add constraint chat_rooms_direct_pair_check
  check ((kind = 'direct' and direct_pair is not null and visibility = 'private')
      or (kind <> 'direct' and direct_pair is null));
alter table public.chat_members add column role text not null default 'member'
  check (role in ('owner', 'admin', 'member', 'subscriber'));
update public.chat_members m set role = 'owner' from public.chat_rooms r
  where r.id = m.room_id and r.owner_id = m.user_id;
create unique index chat_members_one_owner on public.chat_members(room_id) where role = 'owner';

create function chat_private.member_role(target_room uuid) returns text
language sql volatile security definer set search_path = '' as $$
  select role from public.chat_members where room_id = target_room and user_id = auth.uid();
$$;
create function chat_private.can_post(target_room uuid) returns boolean
language sql volatile security definer set search_path = '' as $$
  select exists (select 1 from public.chat_rooms r join public.chat_members m on m.room_id = r.id
    where r.id = target_room and m.user_id = auth.uid()
      and (m.role in ('owner', 'admin') or (r.kind <> 'channel' and m.role = 'member')));
$$;
create function chat_private.can_see_member(target_room uuid,target_user uuid) returns boolean
language sql stable security definer set search_path = '' as $$
  select exists(select 1 from public.chat_rooms r join public.chat_members caller on caller.room_id = r.id
    join public.chat_members person on person.room_id = r.id
    where r.id = target_room and caller.user_id = auth.uid() and person.user_id = target_user
      and (r.kind <> 'channel' or caller.role in ('owner','admin')
        or person.role in ('owner','admin') or person.user_id = auth.uid()));
$$;
drop policy members_member_select on public.chat_members;
create policy members_member_select on public.chat_members for select to authenticated
  using(chat_private.can_see_member(room_id,user_id));
create function public.role(public.chat_rooms) returns text language sql volatile
security definer set search_path = '' as $$ select chat_private.member_role(($1).id); $$;
create function public.can_post(public.chat_rooms) returns boolean language sql volatile
security definer set search_path = '' as $$ select chat_private.can_post(($1).id); $$;
-- Fresh snapshots are required when these selectors run after mutations in an RPC.
create function chat_private.room_json(target_room uuid) returns jsonb language sql volatile
security definer set search_path = '' as $$
  select to_jsonb(r) || jsonb_build_object('role', chat_private.member_role(r.id),
    'can_post', chat_private.can_post(r.id)) from public.chat_rooms r
    where r.id = target_room and chat_private.is_member(r.id);
$$;

drop function public.create_chat_room(text, uuid[]);
create function public.create_chat_room(room_name text, member_ids uuid[],
  room_kind text default 'group', room_visibility text default 'private')
returns setof jsonb language plpgsql security definer set search_path = '' as $$
declare caller uuid := auth.uid(); created public.chat_rooms; peer uuid; pair text;
begin
  if caller is null then raise exception 'Sign in first' using errcode = '28000'; end if;
  if room_kind not in ('direct','group','channel') or room_visibility not in ('private','public')
      or room_kind is null or room_visibility is null then
    raise exception 'Invalid conversation type or visibility' using errcode = '22023'; end if;
  if member_ids is null or cardinality(member_ids) > 50 or array_position(member_ids,null) is not null then
    raise exception 'Choose at most 50 people' using errcode = '22023'; end if;
  if room_name is null or char_length(btrim(room_name)) not between 1 and 80 then
    raise exception 'Conversation name must be 1–80 characters' using errcode = '22023'; end if;
  if room_kind = 'direct' then
    if room_visibility <> 'private' or cardinality(member_ids) <> 1 or member_ids[1] = caller then
      raise exception 'Choose exactly one other person' using errcode = '22023'; end if;
    peer := member_ids[1];
    pair := least(caller::text,peer::text) || ':' || greatest(caller::text,peer::text);
    -- A transaction lock serializes creation of this pair, including membership.
    perform pg_catalog.pg_advisory_xact_lock(pg_catalog.hashtextextended(pair,0));
    select * into created from public.chat_rooms where direct_pair = pair;
    if found then
      if not chat_private.is_member(created.id) then raise exception 'Conversation unavailable' using errcode = '42501'; end if;
      return next chat_private.room_json(created.id); return;
    end if;
  end if;
  if exists (select 1 from unnest(member_ids) id where not exists (select 1 from auth.users u where u.id = id)) then
    raise exception 'One or more people are unavailable' using errcode = '22023'; end if;
  insert into public.chat_rooms(name,kind,visibility,owner_id,direct_pair)
    values(btrim(room_name),room_kind,room_visibility,caller,pair) returning * into created;
  insert into public.chat_members(room_id,user_id,role) values(created.id,caller,'owner');
  insert into public.chat_members(room_id,user_id,role)
    select created.id,id,case when room_kind = 'channel' then 'subscriber' else 'member' end
      from unnest(member_ids) id where id <> caller on conflict do nothing;
  return next chat_private.room_json(created.id);
end; $$;

-- Profiles expose only public names through authorized relationships. Login
-- identifiers are read from Auth only inside self-profile and opt-in lookup RPCs.
alter table public.chat_profiles
  add column username text check (username is null or username ~ '^[a-z][a-z0-9_]{2,31}$'),
  add column discoverable_phone boolean not null default false,
  add column discoverable_email boolean not null default false;
create unique index chat_profiles_username_unique on public.chat_profiles(lower(username)) where username is not null;
-- Auth creates the initial display name. Later identity linking or metadata
-- refreshes must not overwrite the profile edited through update_chat_profile.
create or replace function chat_private.sync_profile() returns trigger language plpgsql
security definer set search_path = '' as $$
begin
  insert into public.chat_profiles(id,display_name)
    values(new.id,coalesce(nullif(btrim(new.raw_user_meta_data->>'display_name'),''),'Member'))
    on conflict(id) do nothing;
  return new;
end; $$;
create function chat_private.shares_room(target_user uuid) returns boolean language sql stable
security definer set search_path = '' as $$
  select exists (select 1 from public.chat_members a join public.chat_members b using(room_id)
    join public.chat_rooms r on r.id = a.room_id where a.user_id = auth.uid() and b.user_id = target_user
      and (r.kind <> 'channel' or a.role in ('owner','admin') or b.role in ('owner','admin')));
$$;
drop policy profiles_directory_select on public.chat_profiles;
create policy profiles_relationship_select on public.chat_profiles for select to authenticated
  using (id = auth.uid() or chat_private.shares_room(id));
grant select(username) on public.chat_profiles to authenticated;

create function public.get_chat_profile() returns jsonb language sql volatile
security definer set search_path = '' as $$
  select to_jsonb(p) || jsonb_build_object(
    'phone',case when u.phone_confirmed_at is not null and nullif(u.phone,'') is not null
      then '+' || regexp_replace(u.phone,'[^0-9]','','g') end,
    'email',case when u.email_confirmed_at is not null then u.email end)
  from public.chat_profiles p join auth.users u on u.id = p.id where p.id = auth.uid();
$$;
create function public.update_chat_profile("values" jsonb) returns jsonb language plpgsql
security definer set search_path = '' as $$
declare u auth.users; p public.chat_profiles; uname text;
begin
  select * into u from auth.users where id = auth.uid();
  if not found then raise exception 'Sign in first' using errcode = '28000'; end if;
  if "values" is null or jsonb_typeof("values") <> 'object' then
    raise exception 'Provide profile fields' using errcode = '22023'; end if;
  select * into p from public.chat_profiles where id = u.id for update;
  uname := case when "values" ? 'username' then nullif(lower(btrim("values"->>'username')),'') else p.username end;
  if uname is not null and uname !~ '^[a-z][a-z0-9_]{2,31}$' then
    raise exception 'Username must be 3–32 letters, numbers or underscores, starting with a letter' using errcode = '22023'; end if;
  if "values" ? 'display_name' and ("values"->>'display_name' is null or char_length(btrim("values"->>'display_name')) not between 1 and 80) then
    raise exception 'Display name must be 1–80 characters' using errcode = '22023'; end if;
  if "values" ? 'discoverable_phone' and coalesce(("values"->>'discoverable_phone')::boolean,false)
      and (u.phone_confirmed_at is null or nullif(u.phone,'') is null) then
    raise exception 'Verify a phone number first' using errcode = '22023'; end if;
  if "values" ? 'discoverable_email' and coalesce(("values"->>'discoverable_email')::boolean,false) and u.email_confirmed_at is null then
    raise exception 'Verify an email address first' using errcode = '22023'; end if;
  update public.chat_profiles set username = uname,
    display_name = case when "values" ? 'display_name' then btrim("values"->>'display_name') else display_name end,
    discoverable_phone = case when "values" ? 'discoverable_phone' then coalesce(("values"->>'discoverable_phone')::boolean,false) else discoverable_phone end,
    discoverable_email = case when "values" ? 'discoverable_email' then coalesce(("values"->>'discoverable_email')::boolean,false) else discoverable_email end
    where id = u.id;
  return public.get_chat_profile();
exception when unique_violation then raise exception 'Username is unavailable' using errcode = '23505';
end; $$;

create table chat_private.lookup_limits(user_id uuid primary key references auth.users(id) on delete cascade,
  window_start timestamptz not null, hits integer not null);
revoke all on chat_private.lookup_limits from public,anon,authenticated;
create function public.find_chat_people(query text)
returns table(id uuid,display_name text,username text) language plpgsql security definer set search_path = '' as $$
declare caller uuid := auth.uid(); attempts integer; needle text := lower(btrim(query));
begin
  if caller is null then raise exception 'Sign in first' using errcode = '28000'; end if;
  insert into chat_private.lookup_limits values(caller,clock_timestamp(),1)
    on conflict(user_id) do update set
      hits = case when chat_private.lookup_limits.window_start < clock_timestamp()-interval '1 minute' then 1 else chat_private.lookup_limits.hits+1 end,
      window_start = case when chat_private.lookup_limits.window_start < clock_timestamp()-interval '1 minute' then clock_timestamp() else chat_private.lookup_limits.window_start end
    returning hits into attempts;
  if attempts > 20 then raise exception 'Too many lookups. Please wait a minute.' using errcode = 'P0001'; end if;
  if needle is null or char_length(needle) not between 3 and 254 then return; end if;
  return query select p.id,p.display_name,p.username from public.chat_profiles p join auth.users u on u.id = p.id
    where p.id <> caller and (
      p.username = case when left(needle,1) = '@' then substr(needle,2) else needle end
      or (p.discoverable_email and u.email_confirmed_at is not null and lower(u.email) = needle)
      or (p.discoverable_phone and u.phone_confirmed_at is not null and needle ~ '^\+[1-9][0-9]{7,14}$'
        and regexp_replace(u.phone,'[^0-9]','','g') = substr(needle,2))) limit 5;
end; $$;

create function public.list_chat_members(target_room uuid)
returns table(user_id uuid,display_name text,username text,role text) language plpgsql
security definer set search_path = '' as $$
begin
  if not chat_private.is_member(target_room) then raise exception 'Conversation unavailable' using errcode = '42501'; end if;
  return query select m.user_id,p.display_name,p.username,m.role from public.chat_members m
    join public.chat_profiles p on p.id = m.user_id where m.room_id = target_room
      and chat_private.can_see_member(target_room,m.user_id) order by m.role,p.display_name,m.user_id;
end; $$;
create function public.set_chat_member_role(target_room uuid,target_user uuid,member_role text)
returns boolean language plpgsql security definer set search_path = '' as $$
declare r public.chat_rooms; current_role text;
begin
  select * into r from public.chat_rooms where id = target_room for update;
  if r.id is null or r.kind = 'direct' or chat_private.member_role(target_room) <> 'owner'
      or chat_private.member_role(target_room) is null then
    raise exception 'Only the owner can change roles' using errcode = '42501'; end if;
  select role into current_role from public.chat_members where room_id = target_room and user_id = target_user for update;
  if current_role is null or current_role = 'owner' or member_role is null
      or member_role not in ('admin','member','subscriber')
      or (r.kind = 'channel' and member_role = 'member') or (r.kind <> 'channel' and member_role = 'subscriber') then
    raise exception 'Invalid membership role' using errcode = '22023'; end if;
  update public.chat_members set role = member_role where room_id = target_room and user_id = target_user;
  return true;
end; $$;
create function public.remove_chat_member(target_room uuid,target_user uuid) returns boolean
language plpgsql security definer set search_path = '' as $$
declare r public.chat_rooms; actor text; victim text;
begin
  select * into r from public.chat_rooms where id = target_room for update;
  actor := chat_private.member_role(target_room);
  select role into victim from public.chat_members where room_id = target_room and user_id = target_user for update;
  if r.id is null or r.kind = 'direct' or actor is null or actor not in ('owner','admin')
      or victim is null or victim = 'owner' or (actor = 'admin' and victim = 'admin') then
    raise exception 'You cannot remove this member' using errcode = '42501'; end if;
  delete from public.chat_members where room_id = target_room and user_id = target_user;
  return true;
end; $$;

create table chat_private.invites(
  code uuid primary key default gen_random_uuid(), room_id uuid not null references public.chat_rooms(id) on delete cascade,
  created_by uuid not null references auth.users(id), expires_at timestamptz not null default now()+interval '24 hours',
  max_uses integer not null default 10 check(max_uses between 1 and 100), uses integer not null default 0,
  revoked boolean not null default false, created_at timestamptz not null default now());
revoke all on chat_private.invites from public,anon,authenticated;
create function public.create_chat_invite(target_room uuid) returns jsonb language plpgsql
security definer set search_path = '' as $$
declare invite chat_private.invites;
begin
  perform 1 from public.chat_rooms where id = target_room and kind <> 'direct' for update;
  if not found or coalesce(chat_private.member_role(target_room),'') not in ('owner','admin') then
    raise exception 'Only admins can invite people' using errcode = '42501'; end if;
  insert into chat_private.invites(room_id,created_by) values(target_room,auth.uid()) returning * into invite;
  return to_jsonb(invite)-'created_by';
end; $$;
create function public.join_chat_invite(invite_code text) returns jsonb language plpgsql
security definer set search_path = '' as $$
declare invite chat_private.invites; r public.chat_rooms;
begin
  if auth.uid() is null then raise exception 'Sign in first' using errcode = '28000'; end if;
  select * into invite from chat_private.invites where code::text = invite_code;
  if invite.code is null then raise exception 'Invitation is unavailable or expired' using errcode = '22023'; end if;
  select * into r from public.chat_rooms where id = invite.room_id for update;
  -- Room first, invite second: the same lock order as revocation.
  select * into invite from chat_private.invites where code::text = invite_code for update;
  if invite.code is null or invite.revoked or invite.expires_at <= now() or invite.uses >= invite.max_uses then
    raise exception 'Invitation is unavailable or expired' using errcode = '22023'; end if;
  if r.kind = 'direct' or coalesce(chat_private.member_role(r.id),'') in ('owner','admin','member','subscriber') then
    if r.kind <> 'direct' then return chat_private.room_json(r.id); end if;
    raise exception 'Invitation unavailable' using errcode = '22023'; end if;
  insert into public.chat_members(room_id,user_id,role)
    values(r.id,auth.uid(),case when r.kind = 'channel' then 'subscriber' else 'member' end);
  update chat_private.invites set uses = uses+1 where code = invite.code;
  return chat_private.room_json(r.id);
end; $$;
create function public.revoke_chat_invite(target_room uuid,invite_code text) returns boolean
language plpgsql security definer set search_path = '' as $$
begin
  perform 1 from public.chat_rooms where id = target_room for update;
  if coalesce(chat_private.member_role(target_room),'') not in ('owner','admin') then
    raise exception 'Only admins can revoke invitations' using errcode = '42501'; end if;
  update chat_private.invites set revoked = true where room_id = target_room and code::text = invite_code;
  return found;
end; $$;

create function public.find_chat_channels(query text) returns setof jsonb language sql stable
security definer set search_path = '' as $$
  select jsonb_build_object('id',r.id,'name',r.name,'kind',r.kind,'visibility',r.visibility,
    'role',chat_private.member_role(r.id),'can_post',chat_private.can_post(r.id))
    from public.chat_rooms r where auth.uid() is not null and r.kind = 'channel' and r.visibility = 'public'
      and char_length(btrim(query)) between 2 and 80 and strpos(lower(r.name),lower(btrim(query))) > 0
    order by r.name,r.id limit 20;
$$;
create function public.join_public_chat_channel(target_room uuid) returns jsonb language plpgsql
security definer set search_path = '' as $$
begin
  perform 1 from public.chat_rooms where id = target_room and kind = 'channel' and visibility = 'public' for update;
  if not found or auth.uid() is null then raise exception 'Channel unavailable' using errcode = '42501'; end if;
  insert into public.chat_members(room_id,user_id,role) values(target_room,auth.uid(),'subscriber') on conflict do nothing;
  return chat_private.room_json(target_room);
end; $$;

create table public.chat_attachments(
  id uuid primary key default gen_random_uuid(), room_id uuid not null references public.chat_rooms(id) on delete cascade,
  uploader_id uuid not null references auth.users(id), message_id uuid references public.chat_messages(id) on delete cascade,
  object_key text not null unique, thumbnail_key text not null unique,
  kind text not null default 'photo' check(kind = 'photo'), mime_type text not null default 'image/jpeg' check(mime_type = 'image/jpeg'),
  width integer not null check(width between 1 and 1600), height integer not null check(height between 1 and 1600),
  byte_count integer not null check(byte_count between 1 and 1000000),
  thumbnail_width integer not null check(thumbnail_width between 1 and 256),
  thumbnail_height integer not null check(thumbnail_height between 1 and 256),
  thumbnail_byte_count integer not null check(thumbnail_byte_count between 1 and 1000000),
  status text not null default 'pending' check(status in ('pending','ready','attached','discarded')),
  created_at timestamptz not null default now(),
  check((status = 'attached') = (message_id is not null)));
create index chat_attachments_message_idx on public.chat_attachments(message_id);
alter table public.chat_attachments enable row level security;
revoke all on public.chat_attachments from public,anon,authenticated;
grant select on public.chat_attachments to authenticated;
grant all on public.chat_attachments to service_role;
create policy attachments_authorized_select on public.chat_attachments for select to authenticated
  using(chat_private.is_member(room_id) and (status = 'attached' or (uploader_id = auth.uid() and status in ('pending','ready'))));
create function public.attachments(public.chat_messages) returns jsonb language sql volatile
security invoker set search_path = '' as $$
  select coalesce(jsonb_agg(to_jsonb(a) order by a.created_at,a.id),'[]'::jsonb)
    from public.chat_attachments a where a.message_id = ($1).id and a.status = 'attached';
$$;
create function public.reserve_chat_photo(target_room uuid,photo_width integer,photo_height integer,photo_bytes integer,
  thumbnail_width integer,thumbnail_height integer,thumbnail_bytes integer) returns jsonb
language plpgsql security definer set search_path = '' as $$
declare a public.chat_attachments; attachment_id uuid := gen_random_uuid(); caller uuid := auth.uid();
begin
  if caller is not null then
    perform pg_catalog.pg_advisory_xact_lock(pg_catalog.hashtextextended('photo:'||caller::text,0));
  end if;
  perform 1 from public.chat_rooms where id = target_room for update;
  if not chat_private.can_post(target_room) then raise exception 'You cannot upload here' using errcode = '42501'; end if;
  if (select count(*) from public.chat_attachments where uploader_id = caller and status in ('pending','ready')
      and created_at > now()-interval '1 hour') >= 20 then
    raise exception 'Too many unfinished photo uploads' using errcode = 'P0001'; end if;
  insert into public.chat_attachments(id,room_id,uploader_id,object_key,thumbnail_key,width,height,byte_count,
    thumbnail_width,thumbnail_height,thumbnail_byte_count)
    values(attachment_id,target_room,caller,target_room||'/'||attachment_id||'/photo.jpg',
      target_room||'/'||attachment_id||'/thumbnail.jpg',photo_width,photo_height,photo_bytes,
      thumbnail_width,thumbnail_height,thumbnail_bytes) returning * into a;
  return to_jsonb(a);
end; $$;

alter table public.chat_messages drop constraint chat_messages_body_check;
alter table public.chat_messages add constraint chat_messages_body_check check(octet_length(body) <= 4000);
revoke insert on public.chat_messages from public,anon,authenticated;
revoke insert(room_id,sender_id,body) on public.chat_messages from public,anon,authenticated;
drop policy messages_member_insert on public.chat_messages;
create policy messages_member_insert on public.chat_messages for insert to authenticated
  with check(sender_id = auth.uid() and chat_private.can_post(room_id) and body ~ '[^[:space:]]');
create function public.post_chat_message(target_room uuid,message_body text,attachment_ids uuid[] default '{}')
returns setof jsonb language plpgsql security definer set search_path = '' as $$
declare m public.chat_messages; requested integer := coalesce(cardinality(attachment_ids),0); ready_count integer;
begin
  -- All membership mutations lock the room too, so removal/demotion cannot race a post.
  perform 1 from public.chat_rooms where id = target_room for update;
  if not chat_private.can_post(target_room) then raise exception 'You cannot publish here' using errcode = '42501'; end if;
  if message_body is null or octet_length(message_body) > 4000 or requested > 4
      or array_position(attachment_ids,null) is not null
      or (message_body !~ '[^[:space:]]' and requested = 0) then
    raise exception 'Write a message or attach up to four photos' using errcode = '22023'; end if;
  perform 1 from public.chat_attachments where room_id = target_room and uploader_id = auth.uid() and id = any(attachment_ids) for update;
  select count(*) into ready_count from public.chat_attachments where id = any(attachment_ids)
    and room_id = target_room and uploader_id = auth.uid() and status = 'ready' and message_id is null
    and created_at > now()-interval '1 hour';
  if ready_count <> requested then raise exception 'Photo is unavailable or unfinished' using errcode = '22023'; end if;
  insert into public.chat_messages(room_id,sender_id,body) values(target_room,auth.uid(),message_body) returning * into m;
  update public.chat_attachments set message_id = m.id,status = 'attached' where id = any(attachment_ids);
  return next to_jsonb(m) || jsonb_build_object('attachments',public.attachments(m));
end; $$;
create function public.discard_chat_photo(attachment_id uuid) returns jsonb language plpgsql
security definer set search_path = '' as $$
declare a public.chat_attachments;
begin
  select * into a from public.chat_attachments where id = attachment_id for update;
  if auth.uid() is null or a.id is null or a.uploader_id is distinct from auth.uid() or a.message_id is not null or a.status = 'attached' then
    raise exception 'Photo cannot be discarded' using errcode = '42501'; end if;
  update public.chat_attachments set status = 'discarded' where id = a.id;
  return to_jsonb(a) || jsonb_build_object('status','discarded');
end; $$;

-- Only the trusted finalizer may call this, after decoding and validating both
-- immutable storage objects against the reserved byte counts and dimensions.
create function public.mark_chat_photo_ready(attachment_id uuid,uploader uuid) returns jsonb
language plpgsql security definer set search_path = '' as $$
declare a public.chat_attachments; target_room uuid;
begin
  select room_id into target_room from public.chat_attachments where id = attachment_id;
  perform 1 from public.chat_rooms where id = target_room for update;
  select * into a from public.chat_attachments where id = attachment_id for update;
  if a.id is null or uploader is null or a.uploader_id <> uploader or a.message_id is not null
      or a.status not in ('pending','ready') or a.created_at <= now()-interval '1 hour'
      or not exists (select 1 from public.chat_members m join public.chat_rooms r on r.id = m.room_id
        where m.room_id = a.room_id and m.user_id = uploader
          and (m.role in ('owner','admin') or (r.kind <> 'channel' and m.role = 'member'))) then
    raise exception 'Photo is unavailable or publishing permission was removed' using errcode = '42501'; end if;
  update public.chat_attachments set status = 'ready' where id = a.id returning * into a;
  return to_jsonb(a);
end; $$;
revoke all on function public.mark_chat_photo_ready(uuid,uuid) from public,anon,authenticated;
grant execute on function public.mark_chat_photo_ready(uuid,uuid) to service_role;

-- The cleanup worker deletes these objects through Storage API, then removes
-- only their discarded, unlinked metadata. Retrying discarded rows is safe.
create function public.claim_expired_chat_photos(batch_limit integer default 100)
returns setof public.chat_attachments language sql volatile security definer set search_path = '' as $$
  with expired as (
    select id from public.chat_attachments
      where message_id is null and status in ('pending','ready','discarded')
        and created_at <= now()-interval '1 hour'
      order by created_at,id
      limit least(greatest(coalesce(batch_limit,100),1),100)
      for update skip locked
  )
  update public.chat_attachments a set status = 'discarded' from expired
    where a.id = expired.id returning a.*;
$$;
revoke all on function public.claim_expired_chat_photos(integer) from public,anon,authenticated;
grant execute on function public.claim_expired_chat_photos(integer) to service_role;

insert into storage.buckets(id,name,public,file_size_limit,allowed_mime_types)
  values('chat-photos','chat-photos',false,1000000,array['image/jpeg'])
  on conflict(id) do update set public=false,file_size_limit=1000000,allowed_mime_types=array['image/jpeg'];
create function chat_private.photo_object_access(object_name text,operation text) returns boolean
language sql stable security definer set search_path = '' as $$
  select exists(select 1 from public.chat_attachments a
    where object_name in (a.object_key,a.thumbnail_key) and (
      (operation = 'read' and ((a.status = 'attached' and chat_private.is_member(a.room_id))
        or (a.uploader_id = auth.uid() and a.message_id is null
          and ((a.status in ('pending','ready') and chat_private.can_post(a.room_id)) or a.status = 'discarded'))))
      or (operation = 'write' and a.uploader_id = auth.uid() and a.status = 'pending'
        and a.created_at > now()-interval '1 hour' and chat_private.can_post(a.room_id))
      or (operation = 'delete' and a.uploader_id = auth.uid() and a.status = 'discarded' and a.message_id is null)));
$$;
create policy chat_photos_read on storage.objects for select to authenticated
  using(bucket_id = 'chat-photos' and chat_private.photo_object_access(name,'read'));
create policy chat_photos_upload on storage.objects for insert to authenticated
  with check(bucket_id = 'chat-photos' and chat_private.photo_object_access(name,'write'));
create policy chat_photos_discard on storage.objects for delete to authenticated
  using(bucket_id = 'chat-photos' and chat_private.photo_object_access(name,'delete'));
-- No object UPDATE policy: uploaded bytes cannot be overwritten after validation.
-- Discarded objects retain uploader-only SELECT for Storage API deletion lookup;
-- they never become timeline attachments or become readable by other members.
-- An administrator cleanup job removes stale pending/ready/discarded objects via
-- Storage API first, then their metadata; deleting storage.objects in SQL leaks bytes.

revoke all on function chat_private.member_role(uuid),chat_private.can_post(uuid),chat_private.room_json(uuid),
  chat_private.shares_room(uuid),chat_private.can_see_member(uuid,uuid),chat_private.photo_object_access(text,text) from public,anon,authenticated;
grant execute on function chat_private.member_role(uuid),chat_private.can_post(uuid),chat_private.shares_room(uuid),
  chat_private.can_see_member(uuid,uuid),chat_private.photo_object_access(text,text) to authenticated;
revoke all on function public.role(public.chat_rooms),public.can_post(public.chat_rooms),public.attachments(public.chat_messages),
  public.create_chat_room(text,uuid[],text,text),public.get_chat_profile(),public.update_chat_profile(jsonb),
  public.find_chat_people(text),public.list_chat_members(uuid),public.set_chat_member_role(uuid,uuid,text),
  public.remove_chat_member(uuid,uuid),public.create_chat_invite(uuid),public.join_chat_invite(text),
  public.find_chat_channels(text),public.join_public_chat_channel(uuid),
  public.revoke_chat_invite(uuid,text),public.reserve_chat_photo(uuid,integer,integer,integer,integer,integer,integer),
  public.post_chat_message(uuid,text,uuid[]),public.discard_chat_photo(uuid) from public,anon;
grant execute on function public.role(public.chat_rooms),public.can_post(public.chat_rooms),public.attachments(public.chat_messages),
  public.create_chat_room(text,uuid[],text,text),public.get_chat_profile(),public.update_chat_profile(jsonb),
  public.find_chat_people(text),public.list_chat_members(uuid),public.set_chat_member_role(uuid,uuid,text),
  public.remove_chat_member(uuid,uuid),public.create_chat_invite(uuid),public.join_chat_invite(text),
  public.find_chat_channels(text),public.join_public_chat_channel(uuid),
  public.revoke_chat_invite(uuid,text),public.reserve_chat_photo(uuid,integer,integer,integer,integer,integer,integer),
  public.post_chat_message(uuid,text,uuid[]),public.discard_chat_photo(uuid) to authenticated;
notify pgrst,'reload schema';
commit;
