import { AppwriteRest, Failure, Repository } from './rest.js';
import { Auth } from './auth.js';
import { ChatService } from './service.js';

export function runtimeIdentity(headers) {
  const jwt = headers?.['x-appwrite-user-jwt'], userId = headers?.['x-appwrite-user-id'];
  if (typeof jwt !== 'string' || !jwt || typeof userId !== 'string' || !userId) {
    throw new Failure(401, 'unauthenticated', 'Execute this function as a signed-in Appwrite user.');
  }
  return { jwt, userId };
}

export async function execute(envelope, service, identity) {
  try {
    const result = await service.dispatch(envelope, identity);
    return Buffer.isBuffer(result) ? { status: 200, body: result.toString('base64'), encoding: 'base64' } : { status: 200, body: result };
  } catch (error) {
    if (error instanceof Failure) return { status: error.status, body: { code: error.code, message: error.message } };
    return { status: 503, body: { code: 'service_unavailable', message: 'The chat service is temporarily unavailable. Please retry.' } };
  }
}

export default async ({ req, res, error: reportError = () => {} }) => {
  try {
    const envelope = req.bodyJson ?? JSON.parse(req.bodyText || req.bodyRaw || '{}');
    // Only platform-authenticated execution headers supply identity. Request JSON
    // (including legacy session/jwt fields) never selects or impersonates a caller.
    const identity = runtimeIdentity(req.headers);
    const apiKey = req.headers['x-appwrite-key'];
    if (!apiKey) return res.json({ status: 503, body: { code: 'configuration', message: 'The function dynamic API key is unavailable.' } });
    const rest = new AppwriteRest({ endpoint: process.env.APPWRITE_FUNCTION_API_ENDPOINT || 'https://fra.cloud.appwrite.io/v1',
      projectId: process.env.APPWRITE_FUNCTION_PROJECT_ID || '69f1c06800389dc6a1a0', apiKey });
    const repository = new Repository(rest, process.env.CHAT_DATABASE_ID || 'octopus-db', process.env.CHAT_BUCKET_ID || 'post-images');
    const auth = new Auth(rest, repository, { otpHmacKey: process.env.CHAT_OTP_HMAC_KEY,
      emailProviderId: process.env.CHAT_EMAIL_PROVIDER_ID, smsProviderId: process.env.CHAT_SMS_PROVIDER_ID });
    return res.json(await execute(envelope, new ChatService(repository, auth, {
      pushEnabled: process.env.CHAT_PUSH_ENABLED === 'true', pushError: reportError,
    }), identity));
  } catch (error) {
    if (error instanceof Failure) return res.json({ status: error.status, body: { code: error.code, message: error.message } });
    return res.json({ status: 400, body: { code: 'invalid_request', message: 'Provide a valid Chat request envelope.' } });
  }
};
