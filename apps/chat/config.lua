-- Public project credentials only. User sessions stay in memory.
return {
    url = os.getenv("SUPABASE_URL"),
    publishableKey = os.getenv("SUPABASE_PUBLISHABLE_KEY"),
}
