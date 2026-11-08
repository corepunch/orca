// Manual operator tool. Never imported by the public request handler.
import { pathToFileURL } from 'node:url';
import { writeFile } from 'node:fs/promises';
import { isAbsolute } from 'node:path';
import { AppwriteRest, Repository, eq } from './rest.js';
import { Auth } from './auth.js';
import { ChatService } from './service.js';
import { moderateReport } from './safety.js';
import { decryptPhoto, inspectJpeg } from './photos.js';

export async function readReportedPhoto(service, reportId, attachmentId) {
  const report = await service.repo.get('reports', reportId);
  if (!report?.message_id || !report.evidence?.attachment_ids?.includes(attachmentId))
    throw new Error('This attachment is not evidence in the selected report.');
  const photo = await service.repo.get('attachments', attachmentId);
  if (!photo || photo.message_id !== report.message_id || photo.uploader_id !== report.target_user)
    throw new Error('The reported photo is unavailable or no longer matches its evidence.');
  const bytes = decryptPhoto(await service.repo.getFile(photo.object_key), photo.secret, photo.object_key);
  inspectJpeg(bytes);
  return bytes;
}

export async function runModeration(service, args) {
  const [command, reportId, ...flags] = args;
  if (command === 'list') {
    const reports = await service.repo.list('reports', [eq('status', 'open')]);
    return reports.map(report => ({ id: report.id, reason: report.reason, target_user: report.target_user,
      created_at: report.created_at, has_message: !!report.message_id }));
  }
  if (command === 'show') {
    const report = await service.repo.get('reports', reportId);
    if (!report) throw new Error('Report not found.');
    return report;
  }
  if (command === 'photo') {
    const [attachmentId, output] = flags;
    if (!reportId || !attachmentId || !output || flags.length !== 2 || !isAbsolute(output))
      throw new Error('Usage: moderation.js photo REPORT_ID ATTACHMENT_ID /absolute/private/output.jpg');
    const bytes = await readReportedPhoto(service, reportId, attachmentId);
    // Explicit operator-selected output, private and exclusive. No keys are
    // exported and existing local files cannot be overwritten accidentally.
    await writeFile(output, bytes, { mode: 0o600, flag: 'wx' });
    return { exported: true, path: output };
  }
  if (!['dismiss', 'takedown', 'suspend'].includes(command) || !reportId || flags[0] !== '--confirm')
    throw new Error('Usage: moderation.js list | show REPORT_ID | dismiss|takedown|suspend REPORT_ID --confirm [review notes]');
  return moderateReport(service, reportId, command, flags.slice(1).join(' '));
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  try {
    if (!process.env.APPWRITE_API_KEY) throw new Error('Set a private operator APPWRITE_API_KEY; never use a client token.');
    const rest = new AppwriteRest({ endpoint: process.env.APPWRITE_ENDPOINT || 'https://fra.cloud.appwrite.io/v1',
      projectId: process.env.APPWRITE_PROJECT_ID || '69f1c06800389dc6a1a0', apiKey: process.env.APPWRITE_API_KEY });
    const repo = new Repository(rest, process.env.CHAT_DATABASE_ID || 'octopus-db', process.env.CHAT_BUCKET_ID || 'post-images');
    const service = new ChatService(repo, new Auth(rest, repo));
    console.log(JSON.stringify(await runModeration(service, process.argv.slice(2)), null, 2));
  } catch (error) {
    // Reports contain private evidence. Never print upstream response bodies.
    console.error(error.status ? `Moderation action failed (HTTP ${error.status}).` : error.message);
    process.exitCode = 1;
  }
}
