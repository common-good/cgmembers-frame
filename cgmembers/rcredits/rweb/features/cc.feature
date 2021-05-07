Feature: Credit Card Donation
AS a non-member
I WANT to support Common Good with a credit card donation
SO it can get money from me without me having to sign up

Setup:
  Given members:
  | uid  | fullName | flags   |*
  | .ZZA | Abe One  | ok,confirmed      |
  | .ZZB | Bea Two  | ok,confirmed      |
  | .ZZC | Cor Pub  | ok,confirmed,co   |
  
Scenario: Someone asks to make a credit card donation
  When member "?" visits "cc"
  Then we show "Donate to Common Good" with:
  | Name        |
  | Email       |
  | Postal Code |
  | Donation    |

  Given next captcha is "37"
  When member "?" completes "cc" with:
  | fullName | email | zip   | amtChoice | amount | comment  | cq | ca |*
  | Zee Zot  | z@    | 01026 | 0         | 26     | awesome! | 37 | 74 |
  Then we redirect to "https://www.paypal.com/donate"

Scenario: Someone completes a credit card donation
  When someone visits "cc/op=done&code=%code" where code is:
  | fullName | email | zip   | amount | comment  |*
  | Zee Zot  | z@    | 01026 | 26     | awesome! |
  Then we say "status": "cc thanks|check it out"
