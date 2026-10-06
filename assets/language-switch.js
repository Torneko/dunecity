'use strict';
function updateLanguageLinks(){const [route,query='']=location.hash.slice(1).split('?'),params=new URLSearchParams(query),context=window.getSiteContext?.();if(context){params.set('mod',context.mod);params.set('house',context.house);}const hash='#'+(route||'accueil')+(params.size?'?'+params.toString():'');document.querySelectorAll('[data-language-page]').forEach(link=>{link.href=link.dataset.languagePage+hash;});}
window.addEventListener('hashchange',updateLanguageLinks);
window.addEventListener('DOMContentLoaded',updateLanguageLinks);
document.addEventListener('click',e=>{if(e.target.closest('[data-language-page]'))updateLanguageLinks();});
updateLanguageLinks();
