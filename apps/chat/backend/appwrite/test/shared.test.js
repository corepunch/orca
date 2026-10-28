import test from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { Repository, Failure, eq, key } from '../src/rest.js';
import { migrateShared, LEGACY } from '../src/migrate.js';
import { ChatService } from '../src/service.js';
const schema = JSON.parse(await readFile(new URL('../schema.json', import.meta.url)));
const clone = value => structuredClone(value);
class Store {
  constructor() {
    this.collections = new Map(); this.docs = new Map(); this.calls = []; this.users = new Map();
    this.add('profiles', { attributes: [{key:'username',type:'string',size:128,required:true,status:'available'}], $permissions:['read("any")'], documentSecurity:true });
  }
  add(id, metadata = {}) { this.collections.set(id, {$id:id, $permissions:[], documentSecurity:false, attributes:[], indexes:[], ...clone(metadata)}); this.docs.set(id, new Map()); }
  put(collection, id, data) { this.docs.get(collection).set(id, {$id:id, ...clone(data)}); }
  async request(method, path, {body} = {}) {
    this.calls.push({method,path,body:clone(body)});
    const url = new URL('https://test.invalid' + path), parts = url.pathname.split('/').map(decodeURIComponent);
    if (parts[1] === 'users') { const user=this.users.get(parts[2]); if (!user) throw new Failure(404,'absent','Absent'); return clone(user); }
    const collection = parts[4], metadata=this.collections.get(collection);
    if (!collection && method==='POST') { this.add(body.collectionId, {$permissions:body.permissions,documentSecurity:body.documentSecurity}); return clone(this.collections.get(body.collectionId)); }
    if (!metadata) throw new Failure(404,'absent','Absent');
    if (parts[5] === 'attributes' || parts[5] === 'indexes') {
      const kind=parts[5];
      if (method==='POST') { const row={...body,status:'available', ...(kind==='attributes'?{type:parts[6]}:{})}; metadata[kind].push(row); return clone(row); }
      return clone(metadata[kind].find(row=>row.key===parts[6]));
    }
    if (parts[5] !== 'documents') {
      if (method==='DELETE') { this.collections.delete(collection); this.docs.delete(collection); return true; }
      return clone(metadata);
    }
    const table=this.docs.get(collection), id=parts[6];
    if (method==='GET' && !id) {
      let documents=[...table.values()];
      const queries=url.searchParams.getAll('queries[]').map(JSON.parse);
      for(const query of queries) if(query.method==='equal') documents=documents.filter(row=>query.values.includes(row[query.attribute]));
      const orders=queries.filter(query=>query.method==='orderAsc'||query.method==='orderDesc');
      documents.sort((a,b)=>{
        for(const query of orders) {const compared=String(a[query.attribute]).localeCompare(String(b[query.attribute]));if(compared)return compared*(query.method==='orderDesc'?-1:1);}
        return a.$id.localeCompare(b.$id);
      });
      const cursors=queries.filter(query=>query.method==='cursorAfter'); assert.ok(cursors.length<=1,'one cursor per request');
      if(cursors.length) {
        const at=documents.findIndex(row=>row.$id===cursors[0].values[0]);
        if(at<0)throw new Failure(400,'general_cursor_not_found','Deleted cursor');
        documents=documents.slice(at+1);
      }
      const limit=queries.find(query=>query.method==='limit')?.values[0]||25;documents=documents.slice(0,limit);
      return {documents:clone(documents)};
    }
    if(method==='GET') { if(!table.has(id)) throw new Failure(404,'absent','Absent'); return clone(table.get(id)); }
    if(method==='DELETE') { table.delete(id); return true; }
    if(method==='POST') {
      if(table.has(body.documentId)) throw new Failure(409,'exists','Exists');
      if(collection==='account-directory' && body.data.handle && [...table.values()].some(row=>row.handle===body.data.handle)) throw new Failure(409,'unique','Handle taken');
      this.put(collection,body.documentId,{...body.data,$permissions:body.permissions}); return clone(table.get(body.documentId));
    }
    if(method==='PATCH') {
      if(collection==='account-directory' && body.data.handle && [...table.values()].some(row=>row.$id!==id&&row.handle===body.data.handle)) throw new Failure(409,'unique','Handle taken');
      this.put(collection,id,{...table.get(id),...body.data}); return clone(table.get(id));
    }
    throw new Error('Unexpected request');
  }
}
const writeCount = rest => rest.calls.filter(call=>call.method!=='GET').length;
async function fixture() { const rest=new Store(); await migrateShared(rest); rest.calls=[]; return rest; }

test('shared profile join isolates private identifiers and preserves website fields and permissions', async()=>{
  const rest=await fixture(), repo=new Repository(rest,'octopus-db','post-images');
  rest.put('profiles','legacy-guid',{username:'Website Label',handle:'alice',userId:'legacy-guid',bio:'Preserve me',interests:['art'],avatarId:'avatar', $permissions:['read("any")','update("user:legacy-guid")']});
  rest.put('account-directory','legacy-guid',{handle:'alice',payload:JSON.stringify({handle:'alice',email:'alice@example.invalid',phone:null,discoverable_email:true,discoverable_phone:false}),email:'alice@example.invalid',phone:null});
  const row=await repo.get('profiles','legacy-guid');
  assert.equal(row.display_name,'Website Label'); assert.equal(row.username,'alice'); assert.equal(row.avatar_id,'avatar'); assert.equal(row.email,'alice@example.invalid');
  await repo.update('profiles','legacy-guid',{...row,display_name:'New label',discoverable_email:false});
  const saved=rest.docs.get('profiles').get('legacy-guid');
  assert.equal(saved.username,'New label'); assert.equal(saved.bio,'Preserve me'); assert.equal(saved.avatarId,'avatar'); assert.deepEqual(saved.interests,['art']);
  assert.deepEqual(saved.$permissions,['read("any")','update("user:legacy-guid")']); assert.equal(saved.email,undefined); assert.equal(saved.payload,undefined);
  assert.equal(JSON.parse(rest.docs.get('account-directory').get('legacy-guid').payload).discoverable_email,false);
  assert.equal((await repo.list('profiles',[eq('username','alice')]))[0].id,'legacy-guid');
  assert.equal((await repo.list('profiles',[eq('email','alice@example.invalid')]))[0].id,'legacy-guid');
  assert.ok(rest.calls.every(call=>!call.path.includes('/chat_')));
  assert.throws(()=>repo.path('arbitrary'),/Unknown collection/);
});

test('profile synchronization does not rewrite unchanged shared rows and exposes explicit account-name failure',async()=>{
  const row={id:'alice',display_name:'Alice',username:'alice',avatar_id:null,email:null,phone:null,discoverable_email:false,discoverable_phone:false};
  let updates=0; const repo={lock:async(_,action)=>action(),get:async()=>({...row}),update:async(_,id,value)=>{updates++;return {...value,id};}};
  const service=new ChatService(repo,{rest:{request:async()=>{throw new Failure(503,'offline','Offline');}}});
  await service.profile({$id:'alice',name:'Alice',emailVerification:false,phoneVerification:false}); assert.equal(updates,0);
  await assert.rejects(service.updateProfile({$id:'alice',name:'Alice'},{username:'another'}),error=>error.code==='stable_account_name');
  await assert.rejects(service.updateProfile({$id:'alice',name:'Alice'},{display_name:'New Alice'}),error=>error.code==='profile_sync'); assert.equal(updates,1);
});

test('schema migration is idempotent, verifies without writes, and refuses private permission drift',async()=>{
  const rest=await fixture();
  await migrateShared(rest); assert.equal(writeCount(rest),0);
  await migrateShared(rest,{verify:true}); assert.equal(writeCount(rest),0);
  const publicMetadata=rest.collections.get('profiles'); assert.deepEqual(publicMetadata.$permissions,['read("any")']); assert.equal(publicMetadata.documentSecurity,true);
  rest.collections.get('account-directory').indexes[0].key='existing_handle_index'; await migrateShared(rest,{verify:true});
  rest.collections.get('account-directory').$permissions=['read("any")'];
  await assert.rejects(migrateShared(rest),/Private collection permissions differ/);
});

function legacy(rest) {
  for(const id of Object.keys(LEGACY))rest.add(id);
  rest.users.set('legacy-guid',{$id:'legacy-guid',name:'Account label'});
  rest.put('chat_profiles','legacy-guid',{payload:JSON.stringify({id:'legacy-guid',display_name:'Old Chat label',username:'old_alias',email:'private@example.invalid',phone:null,discoverable_email:true,discoverable_phone:false}),username:'old_alias',email:'private@example.invalid',phone:null});
  rest.put('profiles','legacy-guid',{username:'Website label',userId:'legacy-guid',bio:'Existing bio',avatarId:'avatar', $permissions:['read("any")']});
  rest.put('chat_messages','message-id',{payload:'{"room_id":"room-id","body":"Preserve exact JSON","created_at":"2026-10-20T12:00:00.000Z"}',room_id:'room-id'});
}
test('legacy copy preserves document IDs and payloads, splits private profiles, and only drops explicitly',async()=>{
  const rest=await fixture(); legacy(rest);
  await assert.rejects(migrateShared(rest,{migrateLegacy:true}),/drain/);
  const result=await migrateShared(rest,{migrateLegacy:true,quiesced:true}); assert.equal(result.collections.chat_messages.count,1);
  assert.equal(rest.docs.get('messages').get('message-id').payload,rest.docs.get('chat_messages').get('message-id').payload);
  assert.equal(rest.docs.get('messages').get('message-id').created_at,'2026-10-20T12:00:00.000Z');
  const publicRow=rest.docs.get('profiles').get('legacy-guid'); assert.equal(publicRow.username,'Website label'); assert.equal(publicRow.handle,undefined); assert.equal(rest.docs.get('account-directory').get('legacy-guid').handle,'old_alias'); assert.equal(publicRow.bio,'Existing bio'); assert.equal(publicRow.email,undefined);
  assert.equal(JSON.parse(rest.docs.get('account-directory').get('legacy-guid').payload).email,'private@example.invalid');
  rest.calls=[]; await migrateShared(rest,{migrateLegacy:true,quiesced:true,verify:true}); assert.equal(writeCount(rest),0);
  await migrateShared(rest,{migrateLegacy:true,quiesced:true}); assert.equal(writeCount(rest),0); assert.ok(rest.collections.has('chat_messages'));
  await migrateShared(rest,{migrateLegacy:true,quiesced:true,dropLegacy:true}); assert.ok(!rest.collections.has('chat_messages')); assert.ok(rest.collections.has('messages'));
});

test('migration refuses destination divergence, account handle collisions, unknown profile fields, and active locks',async()=>{
  for(const kind of ['divergence','collision','private-field','lock']) {
    const rest=await fixture(); legacy(rest);
    if(kind==='divergence')rest.put('messages','message-id',{payload:'{}',room_id:'room-id'});
    if(kind==='collision')rest.users.set('old_alias',{$id:'old_alias'});
    if(kind==='private-field') { const doc=rest.docs.get('chat_profiles').get('legacy-guid'); doc.payload=JSON.stringify({...JSON.parse(doc.payload),unknown_private:'retain'}); }
    if(kind==='lock')rest.put('chat_state','lock-id',{kind:'lock',payload:'{"kind":"lock"}'});
    await assert.rejects(migrateShared(rest,{migrateLegacy:true,dropLegacy:true,quiesced:true}),error=>error.code==='migration_conflict');
    assert.ok(rest.collections.has('chat_messages'));
  }
});


test('partial private profile failure is explicit and retry preserves canonical public metadata',async()=>{
  const rest=await fixture(), repo=new Repository(rest,'octopus-db','post-images');
  rest.put('profiles','alice',{userId:'alice',username:'Alice',handle:'alice',bio:'bio',avatarId:'avatar'});
  const request=rest.request.bind(rest); let fail=true;
  rest.request=async(method,path,options)=>{
    if(fail && method==='POST' && path.endsWith('/account-directory/documents')) {fail=false;throw new Failure(503,'offline','Offline');}
    return request(method,path,options);
  };
  const row={display_name:'New Alice',username:'alice',email:'private@example.invalid',discoverable_email:true};
  await assert.rejects(repo.update('profiles','alice',row),error=>error.status===503);
  assert.equal(rest.docs.get('profiles').get('alice').username,'Alice');
  await repo.update('profiles','alice',row);
  const saved=await repo.get('profiles','alice'); assert.equal(saved.email,'private@example.invalid'); assert.equal(saved.display_name,'New Alice');assert.equal(saved.avatar_id,'avatar');
});

test('history applies original message timestamp ordering before limiting the latest page',async()=>{
  let filters, requested;
  const repo={get:async(name)=>name==='rooms'?{id:'room-id',kind:'group'}:{room_id:'room-id',user_id:'alice'},
    list:async(name,queries,limit)=>{assert.equal(name,'messages');filters=queries;requested=limit;return [];}};
  await new ChatService(repo,{}).listMessages({$id:'alice'},'room-id',25);
  assert.equal(requested,26);assert.deepEqual(filters,[eq('room_id','room-id'),{method:'orderDesc',attribute:'created_at'},{method:'orderDesc',attribute:'$id'}]);
});

test('legacy alias claims reject a different native Account ID and private handle index collisions',async()=>{
  const row={id:'legacy-guid',display_name:'Legacy',username:null,avatar_id:null,email:null,phone:null,discoverable_email:false,discoverable_phone:false};
  let writes=0;
  const repo={lock:async(_,action)=>action(),get:async()=>({...row}),update:async()=>{writes++;}};
  const service=new ChatService(repo,{rest:{request:async()=>({$id:'reserved'})}});
  await assert.rejects(service.updateProfile({$id:'legacy-guid',name:'Legacy'},{username:'reserved'}),error=>error.code==='handle_taken'); assert.equal(writes,0);
  const rest=await fixture(), production=new Repository(rest,'octopus-db','post-images');
  rest.put('account-directory','other-guid',{handle:'reserved',payload:'{}'});
  await assert.rejects(production.saveProfile('legacy-guid',{...row,username:'reserved'}),error=>error.code==='handle_taken');
  assert.equal(rest.docs.get('account-directory').has('legacy-guid'),false);
});


test('owner-writable public handle mirrors cannot impersonate a private account handle',async()=>{
  const rest=await fixture(), repo=new Repository(rest,'octopus-db','post-images');
  rest.put('profiles','attacker',{userId:'attacker',username:'Attacker',handle:'victim'});
  rest.put('profiles','victim',{userId:'victim',username:'Victim'});
  rest.put('account-directory','victim',{handle:'victim',payload:JSON.stringify({handle:'victim'})});
  assert.equal((await repo.get('profiles','attacker')).username,null);
  assert.deepEqual((await repo.list('profiles',[eq('username','victim')])).map(row=>row.id),['victim']);
});


async function historyFixture(count=65) {
  const rest=await fixture(), repo=new Repository(rest,'octopus-db','post-images'),service=new ChatService(repo,{});
  rest.put('conversations','room-id',{payload:JSON.stringify({id:'room-id',kind:'group'})});
  rest.put('conversation-members',key('member:room-id:alice'),{room_id:'room-id',user_id:'alice',payload:'{"room_id":"room-id","user_id":"alice"}'});
  for(let index=0;index<count;index++) {
    // Equal timestamps exercise the deterministic ID tie breaker.
    const id='message-'+String(index).padStart(3,'0'),created_at=new Date(1700000000000+Math.floor(index/2)*1000).toISOString();
    rest.put('messages',id,{room_id:'room-id',created_at,payload:JSON.stringify({room_id:'room-id',created_at,body:id,attachment_ids:[]})});
  }
  rest.calls=[];return {rest,repo,service};
}
test('message pages are bounded, chronological, stable across ties and newly inserted messages',async()=>{
  const {rest,service}=await historyFixture();
  const first=await service.pageMessages({$id:'alice'},'room-id');
  assert.equal(first.messages.length,30);assert.equal(first.has_more,true);assert.equal(first.next_cursor,'message-035');
  assert.equal(first.messages[0].id,'message-035');assert.equal(first.messages.at(-1).id,'message-064');
  rest.put('messages','new-message',{room_id:'room-id',created_at:'2026-10-28T10:00:00.000Z',payload:'{"room_id":"room-id","created_at":"2026-10-28T10:00:00.000Z"}'});
  const second=await service.pageMessages({$id:'alice'},'room-id',30,first.next_cursor);
  assert.deepEqual(second.messages.map(row=>row.id),Array.from({length:30},(_,i)=>'message-'+String(i+5).padStart(3,'0')));
  const last=await service.pageMessages({$id:'alice'},'room-id',30,second.next_cursor);
  assert.equal(last.messages.length,5);assert.equal(last.has_more,false);assert.equal(last.next_cursor,null);
  const requests=rest.calls.filter(call=>call.path.includes('/messages/documents?'));
  assert.equal(requests.length,3);for(const call of requests) {
    const params=new URL('https://test.invalid'+call.path).searchParams;assert.equal(params.get('total'),'false');
    assert.deepEqual(JSON.parse(params.getAll('queries[]').find(value=>JSON.parse(value).method==='select')).values,['$id','payload']);
    assert.equal(JSON.parse(new URL('https://test.invalid'+call.path).searchParams.getAll('queries[]').find(value=>JSON.parse(value).method==='limit')).values[0],31);
  }
});
test('maximum pages keep only one cursor while fetching N+1 across repository batches',async()=>{
  const {rest,service}=await historyFixture(230);
  const page=await service.pageMessages({$id:'alice'},'room-id',500,'message-200');
  assert.equal(page.messages.length,100);assert.equal(page.messages[0].id,'message-100');assert.equal(page.messages.at(-1).id,'message-199');assert.equal(page.next_cursor,'message-100');assert.equal(page.has_more,true);
  assert.equal(rest.calls.filter(call=>call.path.includes('/messages/documents?')).length,2);
});
test('stale, wrong-room and concurrently deleted cursors fail explicitly and room access is required',async()=>{
  const {rest,service}=await historyFixture();
  for(const cursor of ['absent','other-room']) {
    if(cursor==='other-room')rest.put('messages',cursor,{room_id:'private-room',payload:'{"room_id":"private-room"}'});
    await assert.rejects(service.pageMessages({$id:'alice'},'room-id',30,cursor),error=>error.status===409&&error.code==='stale_cursor');
  }
  await assert.rejects(service.pageMessages({$id:'outsider'},'room-id',30),error=>error.status===403);
  const request=rest.request.bind(rest);rest.request=async(method,path,options)=>{
    if(path.includes('/messages/documents?'))throw new Failure(400,'general_cursor_not_found','Deleted cursor');
    return request(method,path,options);
  };
  await assert.rejects(service.pageMessages({$id:'alice'},'room-id',30,'message-040'),error=>error.status===409&&error.code==='stale_cursor');
  await assert.rejects(service.pageMessages({$id:'alice'},'room-id',100),error=>error.status===409&&error.code==='stale_cursor');
});
test('page limits clamp to configured bounds and legacy rows keep their descending response',async()=>{
  const {service}=await historyFixture();
  assert.equal((await service.pageMessages({$id:'alice'},'room-id',0)).messages.length,1);
  for(const limit of ['bad',null,1.5,Infinity])await assert.rejects(service.pageMessages({$id:'alice'},'room-id',limit),error=>error.status===400);
  const legacy=await service.listMessages({$id:'alice'},'room-id',2);assert.deepEqual(legacy.map(row=>row.id),['message-064','message-063']);
});
