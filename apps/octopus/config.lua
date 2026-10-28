-- Public project values shared with the Octopus website. User sessions stay in memory.
return {
    backend = os.getenv("OCTOPUS_BACKEND") or "appwrite",
    endpoint = os.getenv("APPWRITE_ENDPOINT") or "https://fra.cloud.appwrite.io/v1",
    projectId = os.getenv("APPWRITE_PROJECT_ID") or "69f1c06800389dc6a1a0",
    databaseId = os.getenv("OCTOPUS_DATABASE_ID") or "octopus-db",
    bucketId = os.getenv("OCTOPUS_BUCKET_ID") or "post-images",
    feedCollectionId = os.getenv("OCTOPUS_FEED_COLLECTION_ID"),
}
