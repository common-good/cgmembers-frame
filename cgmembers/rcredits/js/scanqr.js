import QrScanner from './x/qr-scanner.min.js';
  
QrScanner.WORKER_PATH = 'rcredits/js/x/qr-scanner-worker.min.js';
var front = getCookie('frontCamera');

navigator.mediaDevices.getUserMedia({video: { // constraints
    width: {
      ideal: 1920,
      max: 2560
    },
    height: {
      ideal: 1080,
      max: 1440
    },
    facingMode: (front ? {ideal: 'user'} : {ideal: 'environment'})
}})
  .then(function(localMediaStream) { // successCallback
    $('#edit-result').hide();

    const scanner = new QrScanner(document.getElementById('scanqr'), 
      result => {
        scanner.stop();
        
        if (result === undefined || result == '') { // nothing yet
          scanner.start();

        } else if (has(result, '/')) { // Common Good card
          var slash = result.split('/'); // HTTP:,,DOM.RC2.ME,code
          if (slash !== undefined && slash.length > 3) {
            var dot = slash[2].split('.');
            var uri = '/card/' + dot[0] + '/' + slash[3];

            if (dot[2] == 'ME') {
              if (dot[1] == 'RC2') {location.href = 'https://new.commongood.earth' + uri; throw '';}
              if (dot[1] == 'RC4') {location.href = 'https://demo.commongood.earth' + uri; throw '';}
            }
          }
          $('#edit-result').html('<center><h2>No valid QR code found.</h2><p><a href="' + result + '">' + result + '</a></p></center>').show();

        } else { // admin password
          post('vKeyPw', {vKeyPw:result}, function (j) {
            location.href = ('' + document.location).replace(/scan-qr.*/, '') + 'sadmin/panel/codePw=' + j.codePw;
            throw '';
          });

        }
      },
      error => {if (error != 'No QR code found') alert(error);} // gets called repeatedly with 'No QR code found'
    );

    try {
      scanner.start();
    } catch (e) {alert(e);}
  })
  .catch(function(err) { // errorCallback
    if (has(err, 'denied')) err += '. You cannot scan a Common Good card without allowing this page to use the device\'s camera. To find out how to allow it, search the Internet for "allow camera access" and the name of your browser or device.';
    alert(err);
  });


