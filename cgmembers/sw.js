 /* To clear a buggy service worker, set normal = false;
    This skips over the "waiting" lifecycle state, to ensure the
    new service worker is activated immediately, even if there's
    another tab open controlled by the older service worker code.
    See also the EventListener for 'activate', below.
    */

var normal = false;
var cacheName = 'cgPay';
var filesToCache = [
//  'rcredits/images/favicons/manifest.json',
  'account-thumb',
  'rcredits/images/logo80.png',
  'rcredits/css/x/bootstrap.min.css',
//  'rcredits/css/x/jquery-ui.min.css',
  'rcredits/css/x/ladda-themeless.min.css',
  'rcredits/css/cg.css',
  'rcredits/css/rweb.css',
  'rcredits/js/x/jquery-3.3.1.min.js',
//  'rcredits/js/x/jquery-ui.min.js',
  'rcredits/js/x/bootstrap.min.js',
  'rcredits/js/x/ladda.min.js',
//  'rcredits/js/x/ie10-viewport-bug-workaround.js',
  'rcredits/js/parse-query.js',
  'rcredits/js/misc.js',
  'rcredits/js/scraps.js',
  'rcredits/js/x/qr-scanner.min.js',
  'rcredits/js/x/qr-scanner.min.js.map',
  'rcredits/js/x/qr-scanner-worker.min.js',
  'rcredits/js/x/qr-scanner-worker.min.js.map',
  'app', // start page
  'scan-qr' // scanner
//  'card', // customer id/desc/amount page
//  'card-done',
//  'card-tip',
];

/* Start the service worker and cache content */
self.addEventListener('install', function(e) {
  if (normal) e.waitUntil(
    caches.open(cacheName).then(function(cache) {
      filesToCache.forEach(function (url) {
        try {
          cache.add(url).then(function() {
            console.log('cached ' + url)
          })
        } catch (e) {
          console.log('TypeError caching ' + url + ': ' + e.message);
        }
      });
//      return cache.addAll(filesToCache);
    })
  );
  self.skipWaiting();
});

/* Serve cached content when offline */
if (normal) self.addEventListener('fetch', function(e) {
  e.respondWith(
    caches.match(e.request, {ignoreSearch: true}).then(function(response) { // ignoreSearch=false means query parameters constitute unique pages
      console.log('fetching ' + e.request.url + ' from cache');
      try {
        return response || fetch(e.request);
      } catch (err) {
        console.log('Failed to fetch ' + e.request.url + ' from cache: ' + err.message);
      }
    })
  );
});

self.addEventListener('install', () => {

  self.skipWaiting();
});

/*
self.addEventListener('activate', () => {
  // Optional: Get a list of all the current open windows/tabs under
  // our service worker's control, and force them to reload.
  // This can "unbreak" any open windows/tabs as soon as the new
  // service worker activates, rather than users having to manually reload.
  self.clients.matchAll({type: 'window'}).then(windowClients => {
    windowClients.forEach(windowClient => {
      windowClient.navigate(windowClient.url);
    });
  });
});
*/