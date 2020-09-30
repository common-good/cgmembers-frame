Feature: A company scans a member card with a standard QR app
AS a member company
I WANT to charge a member company or individual by scanning their card on any device
SO I can avoid having multiple devices in my pocket or at the checkout counter

Setup:
  Given members:
  | uid  | fullName | pass | email | flags                    | zip   | floor | cardCode | city | state |*
  | .ZZA | Abe One  | a1   | a@    | member,ok,confirmed,debt | 01001 |  -100 | Aa1      | Aville | AL  |
  | .ZZB | Bea Two  | b1   | b@    | member,ok,confirmed,debt | 01001 |  -100 | Bb2      | Bville | DC  |
  | .ZZC | Our Pub  | c1   | c@    | member,ok,co,confirmed   | 01003 |     0 | Cc3      | Cville | CA  |
  And relations:
  | main | other | permission |*
  | .ZZC | .ZZB  | manage     |
  And member is logged out

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
