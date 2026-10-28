import { configuration, createRepository } from './repository.js';
import { execute } from './materialize.js';

export default async ({ req, res, error }) => {
  try {
    const repo = createRepository(configuration(), req.headers['x-appwrite-key']);
    const result = await execute(repo, { trigger: req.headers['x-appwrite-trigger'],
      event: req.headers['x-appwrite-event'], payload: req.bodyJson || {} });
    if (result.body.failed?.length) error(`Feed reconciliation skipped ${result.body.failed.length} posts; inspect schema and rerun reconciliation.`);
    return res.json(result.body, result.status);
  } catch (failure) {
    error(`Feed materialization failed${failure.status ? ` (${failure.status})` : ''}; inspect source/schema and reconcile.`);
    return res.json({ error: 'Feed materialization failed. Reconciliation required.' }, 500);
  }
};
