-- Public project credentials only. Private sessions use the app's secure store.
return {
    backend = os.getenv("CHAT_BACKEND") or "appwrite",
    endpoint = os.getenv("APPWRITE_ENDPOINT") or "https://fra.cloud.appwrite.io/v1",
    projectId = os.getenv("APPWRITE_PROJECT_ID") or "69f1c06800389dc6a1a0",
    pushProviderId = os.getenv("APPWRITE_PUSH_PROVIDER_ID") or "",
    functionId = os.getenv("APPWRITE_CHAT_FUNCTION_ID") or "orca-chat-api",
}
