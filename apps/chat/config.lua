-- Public project credentials only. User sessions stay in memory.
return {
    backend = os.getenv("CHAT_BACKEND") or "appwrite",
    endpoint = os.getenv("APPWRITE_ENDPOINT") or "https://fra.cloud.appwrite.io/v1",
    projectId = os.getenv("APPWRITE_PROJECT_ID") or "69f1c06800389dc6a1a0",
    functionId = os.getenv("APPWRITE_CHAT_FUNCTION_ID") or "orca-chat-api",
    url = os.getenv("SUPABASE_URL"),
    publishableKey = os.getenv("SUPABASE_PUBLISHABLE_KEY"),
}
