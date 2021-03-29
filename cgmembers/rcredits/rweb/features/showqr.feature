Feature: QR Code for Abe One
AS a member
I WANT to display my account's QR code
SO a seller can scan it and charge me

Setup:
  Given members:
  | uid  | fullName | flags   |*
  | .ZZA | Abe One  | ok,confirmed      |
  | .ZZB | Bea Two  | ok,confirmed      |
  | .ZZC | Cor Pub  | ok,confirmed,co   |
  
Scenario: An online-only member tries to display their QR
  When member ".ZZA" visits "show-qr"
  Then we show "QR Code for Abe One"
  And with:
  | complete |

Scenario: A card member displays their QR
  Given member ".ZZA" has "card" steps done: "all"
  When member ".ZZA" visits "show-qr"
  Then we show "QR Code for Abe One"
  And without:
  | complete |
