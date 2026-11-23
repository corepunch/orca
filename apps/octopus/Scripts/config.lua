-- Public project values shared with the Octopus website. Credentials are stored separately per app.
local feedCollectionId = os.getenv("OCTOPUS_FEED_COLLECTION_ID") or "post-feed"
return {
    -- Enable each provider only after testing the signed iOS build with Chat installed.
    appleSignIn = os.getenv("OCTOPUS_IOS_APPLE_SIGNIN") == "1",
    googleSignIn = os.getenv("OCTOPUS_IOS_GOOGLE_SIGNIN") == "1",
    backend = os.getenv("OCTOPUS_BACKEND") or "appwrite",
    endpoint = os.getenv("APPWRITE_ENDPOINT") or "https://fra.cloud.appwrite.io/v1",
    projectId = os.getenv("APPWRITE_PROJECT_ID") or "69f1c06800389dc6a1a0",
    databaseId = os.getenv("OCTOPUS_DATABASE_ID") or "octopus-db",
    bucketId = os.getenv("OCTOPUS_BUCKET_ID") or "post-images",
    feedCollectionId = feedCollectionId ~= "" and feedCollectionId or nil,
    functionId = os.getenv("OCTOPUS_FUNCTION_ID") or "orca-chat-api",
}
