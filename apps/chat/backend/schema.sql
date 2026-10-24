-- Run once in the Supabase SQL editor of a new project, then run 002_contacts.sql.
begin;

create table public.chat_rooms (
  id uuid primary key default gen_random_uuid(),
  name text not null check (char_length(btrim(name)) between 1 and 80),
  created_at timestamptz not null default now()
);

create table public.chat_members (
  room_id uuid not null references public.chat_rooms(id) on delete cascade,
  user_id uuid not null references auth.users(id) on delete cascade,
  primary key (room_id, user_id)
);
create index chat_members_user_idx on public.chat_members(user_id, room_id);

create table public.chat_messages (
  id uuid primary key default gen_random_uuid(),
  room_id uuid not null references public.chat_rooms(id) on delete cascade,
  sender_id uuid not null references auth.users(id),
  body text not null check (char_length(btrim(body)) > 0 and octet_length(body) <= 4000),
  created_at timestamptz not null default now()
);
create index chat_messages_room_created_idx on public.chat_messages(room_id, created_at desc, id desc);

alter table public.chat_rooms enable row level security;
alter table public.chat_members enable row level security;
alter table public.chat_messages enable row level security;

-- Keep the membership helper outside the exposed public schema. Its owner must
-- be the migration administrator, so checking membership does not recurse RLS.
create schema if not exists chat_private;
revoke all on schema chat_private from public;
grant usage on schema chat_private to authenticated;
create function chat_private.is_member(target_room uuid)
returns boolean language sql stable security definer set search_path = '' as $$
  select exists (
    select 1 from public.chat_members
    where room_id = target_room and user_id = (select auth.uid())
  );
$$;
revoke all on function chat_private.is_member(uuid) from public;
grant execute on function chat_private.is_member(uuid) to authenticated;

create policy rooms_member_select on public.chat_rooms
for select to authenticated using (chat_private.is_member(id));
create policy members_member_select on public.chat_members
for select to authenticated using (chat_private.is_member(room_id));
create policy messages_member_select on public.chat_messages
for select to authenticated using (chat_private.is_member(room_id));
create policy messages_member_insert on public.chat_messages
for insert to authenticated
with check (sender_id = (select auth.uid()) and chat_private.is_member(room_id));

-- All membership/room writes go through this atomic function. There are no
-- client policies for editing membership, deleting rooms, or altering messages.
create function public.create_chat_room(room_name text, member_ids uuid[])
returns setof public.chat_rooms language plpgsql security definer set search_path = '' as $$
declare
  caller uuid := auth.uid();
  created public.chat_rooms;
begin
  if caller is null then raise exception 'Sign in first' using errcode = '28000'; end if;
  if room_name is null or char_length(btrim(room_name)) not between 1 and 80 then
    raise exception 'Conversation name must be 1–80 characters' using errcode = '22023';
  end if;
  if member_ids is null or cardinality(member_ids) < 1 or cardinality(member_ids) > 50
      or array_position(member_ids, null) is not null then
    raise exception 'Choose 1–50 members' using errcode = '22023';
  end if;
  insert into public.chat_rooms(name) values (btrim(room_name)) returning * into created;
  insert into public.chat_members(room_id, user_id) values (created.id, caller);
  insert into public.chat_members(room_id, user_id)
    select created.id, member_id from unnest(member_ids) as member_id
    on conflict do nothing;
  return next created;
end;
$$;
revoke all on function public.create_chat_room(text, uuid[]) from public, anon;
grant execute on function public.create_chat_room(text, uuid[]) to authenticated;

revoke all on public.chat_rooms, public.chat_members, public.chat_messages from anon, authenticated;
grant select on public.chat_rooms, public.chat_members, public.chat_messages to authenticated;
-- Clients cannot spoof server timestamps or message IDs.
grant insert (room_id, sender_id, body) on public.chat_messages to authenticated;

commit;
