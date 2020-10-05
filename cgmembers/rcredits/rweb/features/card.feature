Feature: A company scans a member card with a standard QR app
AS a member company
I WANT to charge a member company or individual by scanning their card on any device
SO I can avoid having multiple devices in my pocket or at the checkout counter

Setup:
  Given members:
  | uid  | fullName | pass | email | flags                    | zip   | floor | cardCode | city | state | cardCode2 |*
  | .ZZA | Abe One  | a1   | a@    | member,ok,confirmed,debt | 01001 |  -100 | Aa1      | Aville | AL  | Aa12      |
  | .ZZB | Bea Two  | b1   | b@    | member,ok,confirmed,debt | 01001 |  -100 | Bb2      | Bville | DC  | Bb22      |
  | .ZZC | Our Pub  | c1   | c@    | member,ok,co,confirmed   | 01003 |     0 |          | Cville | CA  |           |
  And relations:
  | main | other | permission | otherNum |*
  | .ZZC | .ZZB  | manage     | 1        |
  And member is logged out

Scenario: A member scans a company card with a standard QR reader
  Given cookie "scanner" is ""
  When member "?" visits page "card/6VM/LDJK0Bb22"
  Then we show
  | Device Owner Setup | |
  | Associate this device with Our Pub | |
  | No | Yes |
  
  When member "?" confirms form "card/6VM/LDJK0Bb22" with values:
  | op  |*
  | yes |
  Then we say "status": "This device is now associated with Our Pub."  
  And cookie "scanner" is "NEWZZC"
  
  When member "?" visits page "card/6VM/LDJK0Bb22"
  Then we show
  | You: Our Pub | |
  | Do you want to disconnect | |
  | No | Yes |

  When member "?" confirms form "card/6VM/LDJK0Bb22" with values:
  | op  |*
  | yes |
  Then we say "status": "This device is no longer associated with Our Pub."  
  And cookie "scanner" is ""

Scenario: 
  Given members have:
  | uid  | selling   |*
  | .ZZC | groceries |
  And cryptcookie "qid" is "NEWZZB"
  And cookie "scanner" is "NEWZZC"
  And cookie "trust" is "1"
  And member ".ZZA" has picture "picture1"
  When member "?" visits page "card/6vm/KDJIAa1"
  Then we show
  | You: Our Pub ||
  | picture1     ||
  | Abe One      ||
  | Aville, AL   ||
  | Amount       ||
  | For          ||
  | Charge       ||
  And with options:
  | groceries |
  
  When member "?" confirms form "card/6vm/KDJIAa1" with values:
  | op     | amount | desc      |*
  | charge | 10     | groceries |
  Then we say "status": "report tx" with subs:
  | did     | otherName | amount |*
  | charged | Abe One   | $10    |
  And transactions:
  | xid | created | amount | payer | payee | purpose   | taking |*
  |   1 | %today  |     10 | .ZZA  | .ZZC  | groceries | 1      |
