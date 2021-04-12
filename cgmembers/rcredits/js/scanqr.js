import QrScanner from './x/qr-scanner.min.js';
QrScanner.WORKER_PATH = 'rcredits/js/x/qr-scanner-worker.min.js';

// QrScanner.hasCamera().then(hasCamera => camHasCamera.textContent = hasCamera);

const scanner = new QrScanner(document.getElementById('scanqr'), 
  result => {
    scanner.stop();
    $('#scanqr').text('');
    var slash = result.split('/'); // HTTP:,,DOM.RC2.ME,code
    var dot = slash[2].split('.');
    var uri = '/card/' + dot[0] + '/' + slash[3];

    if (dot[2] == 'ME') {
      if (dot[1] == 'RC2') {location.href = 'https://new.commongood.earth' + uri; throw '';}
      if (dot[1] == 'RC4') {location.href = 'https://demo.commongood.earth' + uri; throw '';}
    }
    $('#edit-result').html('<a href="' + result + '">' + result + '</a>');
  },
  error => {
    if (error != 'No QR code found') alert(error); // gets called repeatedly with 'No QR code found'
  }
);

scanner.start();
