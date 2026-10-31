const validId = value => typeof value === 'string' && /^[A-Za-z0-9][A-Za-z0-9._-]{0,35}$/.test(value);
const edge = value => Number.isInteger(value) && value > 0 && value <= 65535;

// Zero is an unknown dimension: Appwrite integer arrays cannot contain nulls.
// Keep each width/height pair at the same position as its file ID.
export function albumFields(post) {
  const imageIds = [], imageWidths = [], imageHeights = [];
  const add = (id, width, height) => {
    if (!validId(id) || imageIds.includes(id) || imageIds.length === 9) return;
    imageIds.push(id);
    const paired = edge(width) && edge(height);
    imageWidths.push(paired ? width : 0);
    imageHeights.push(paired ? height : 0);
  };
  if (Array.isArray(post.imageIds) && post.imageIds.length) {
    post.imageIds.forEach((id, i) => add(id, post.imageWidths?.[i], post.imageHeights?.[i]));
  } else add(post.imageId, post.imageWidth, post.imageHeight);
  return { imageIds, imageWidths, imageHeights };
}
