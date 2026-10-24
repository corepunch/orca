-- Run after schema.sql, including on existing chat projects.
begin;

create table public.chat_profiles (
  id uuid primary key references auth.users(id) on delete cascade,
  display_name text not null default 'Member'
);
alter table public.chat_profiles enable row level security;
revoke all on public.chat_profiles from public, anon, authenticated;
grant select (id, display_name) on public.chat_profiles to authenticated;
create policy profiles_directory_select on public.chat_profiles
for select to authenticated using (id <> (select auth.uid()));

-- Only public display names enter the directory; emails and credentials stay in Auth.
create function chat_private.sync_profile()
returns trigger language plpgsql security definer set search_path = '' as $$
begin
  insert into public.chat_profiles(id, display_name)
  values (new.id, coalesce(nullif(btrim(new.raw_user_meta_data ->> 'display_name'), ''), 'Member'))
  on conflict (id) do update set display_name = excluded.display_name;
  return new;
end;
$$;
revoke all on function chat_private.sync_profile() from public, anon, authenticated;
create trigger chat_sync_profile
after insert or update of raw_user_meta_data on auth.users
for each row execute function chat_private.sync_profile();

insert into public.chat_profiles(id, display_name)
select id, coalesce(nullif(btrim(raw_user_meta_data ->> 'display_name'), ''), 'Member')
from auth.users;

commit;
