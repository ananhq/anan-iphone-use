// SPDX-License-Identifier: Apache-2.0
// Written before the scan UI. Synthetic DOM/responses only, never device access.
const fs=require('node:fs'),vm=require('node:vm'),assert=require('node:assert/strict');
const nodes=new Map(),calls=[];
function node(id='') {
 const n={id,value:'',textContent:'',disabled:false,hidden:false,children:[],dataset:{},
  appendChild(x){this.children.push(x);return x;},replaceChildren(...xs){this.children=[...xs];},
  focus(){this.focused=true;},querySelector(selector){return node(id+selector);}};
 return n;
}
const document={documentElement:{},getElementById(id){if(!nodes.has(id))nodes.set(id,node(id));return nodes.get(id);},
 createElement:tag=>node(tag),querySelectorAll:()=>[]};
const storage={getItem:()=>null,setItem(){throw Error('Scan must not store credentials');},removeItem(){}};
let mode='normal',polls=0;
const strange='<img src=x onerror=alert(1)>';
const context=vm.createContext({document,location:{protocol:'http:',hostname:process.env.SETUP_AP?'192.168.4.1':'192.168.7.1'},
 localStorage:storage,sessionStorage:storage,navigator:{},URL:{},Blob:class{},TextEncoder,
 clearTimeout(){},setTimeout(fn,ms){if(ms===500)fn();return 1;},
 fetch:async(path,opts={})=>{calls.push({path,opts});
  if(path==='/status')return {ok:true,json:async()=>({language:'zh',firmwareVersion:'synthetic',wifiConnected:false,setupAP:!!process.env.SETUP_AP,mcpHost:'stopwatch.local'})};
  if(path==='/wifi/networks')return {ok:true,json:async()=>({networks:[{ssid:strange}]})};
  if(path==='/config')return {ok:true,json:async()=>({mcpServers:{stopwatch:{url:'http://stopwatch.local/mcp'}}})};
  assert.equal(path,'/wifi/scan');
  assert.equal(opts.cache,'no-store');
  if(opts.method==='POST') {polls=0;return {ok:true,json:async()=>({state:'scanning'})};}
  polls++;
  if(mode==='fail')return {ok:true,json:async()=>({state:'failed',error:1,networks:[]})};
  if(mode==='empty')return {ok:true,json:async()=>({state:'ready',networks:[]})};
  if(polls===1)return {ok:true,json:async()=>({state:'scanning'})};
  return {ok:true,json:async()=>({state:'ready',networks:[{ssid:strange,rssi:-40,open:false},{ssid:'Open test',rssi:-60,open:true}]})};
 }});
const html=fs.readFileSync('main/web/setup.html','utf8');
vm.runInContext(html.split('<script>')[1].split('</script>')[0],context);
(async()=>{
 const $=id=>document.getElementById(id);
 await Promise.resolve();await Promise.resolve();
 if(process.env.SETUP_AP){assert.equal($('wifiSetup').hidden,false);assert.equal($('network').value,'wifi');}
 await $('scanWifi').onclick();
 assert.equal($('scanWifi').disabled,false); assert.equal($('saveWifi').disabled,false);
 const options=$('wifiNetworks').children;
 assert.equal(options.length,3); assert.equal(options[1].value,strange);
 assert.ok(options[1].textContent.includes(strange)); // Text, never interpreted markup.
 $('wifiPassword').value='synthetic-only';$('wifiNetworks').value=strange;$('wifiNetworks').onchange();
 assert.equal($('ssid').value,strange);assert.equal($('wifiPassword').value,'');
 assert.equal($('wifiPassword').disabled,false);
 $('wifiNetworks').value='Open test';$('wifiNetworks').onchange();
 assert.equal($('wifiPassword').disabled,true);assert.equal($('wifiPassword').value,'');
 $('ssid').value='Hidden synthetic';$('ssid').oninput();
 assert.equal($('wifiPassword').disabled,false);assert.equal($('wifiNetworks').value,'');
 mode='empty';await $('scanWifi').onclick();assert.equal($('wifiNetworks').children.length,1);
 assert.equal($('ssid').value,'Hidden synthetic');
 mode='fail';await $('scanWifi').onclick();assert.equal($('scanWifi').disabled,false);
 assert.equal($('saveWifi').disabled,false);assert.equal($('forgetWifi').disabled,false);
 assert.ok(!calls.some(c=>JSON.stringify(c.opts).includes('synthetic-only')));
 console.log('Scan page selection, literal SSIDs, manual/open-network and error recovery passed (synthetic).');
})().catch(e=>{console.error(e);process.exitCode=1;});
