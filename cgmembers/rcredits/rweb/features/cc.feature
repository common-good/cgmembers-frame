Feature: Credit Card Donation
AS a non-member
I WANT to support Common Good with a credit card donation
SO it can get money from me without me having to sign up

Setup:
  Given members:
  | uid  | fullName              | flags             | postalAddr            | phone        | legalName      | emailCode |*
  | .ZZA | Abe One               | ok,confirmed      | 1 A, Aville, AL 10001 |              | Abe One        |           |
  | .ZZB | Bea Two               | ok,confirmed      | 2 B, Bville, BC 10002 |              | Bea Two        |           |
  | .ZZC | %PROJECT FBO Cor Pub  | ok,confirmed,co   | 3 C, Cville, CA 10003 | 333-333-3333 | %CGF_LEGALNAME | Cc3       |

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
  Then we say "status": "gift thanks|check it out"
