Feature: A member scans their own card with a company's CGPay app
AS a member
I WANT to pay a member company by scanning my card using the CGPay app on their device
SO I can check out without assistance

Setup:
  Given members:
  | uid  | fullName | email | flags                       | zip   | floor | cardCode | city | state | pass |*
  | .ZZA | Abe One  | a@    | member,ok,confirmed,debt    | 01001 |  -100 | Aa1      | Aville | AL  | 123  |
  | .ZZB | Bea Two  | b@    | member,ok,confirmed,debt    | 01001 |  -100 | Bb2      | Bville | DC  | 123  |
  | .ZZC | Our Pub  | c@    | member,ok,co,confirmed,debt | 01003 |  -100 |          | Cville | CA  |      |
  And member ".ZZA" has picture "picture1"
  And members have:
  | uid  | selling   |*
  | .ZZC | groceries |
  And cookie "selfServe" is "1"
  And cookie "scanner" is "NEWZZC"
  And cookie "box" is "abcd"
  And these "r_boxes":
  | id | channel | code | boxnum | uid  |*
  | 2  | %TX_WEB | abcd | 12345  | .ZZC |

Scenario: A member scans their card
  When member "?" visits "card/6VM/KDJIAa1"
  Then we show
  | Pay: Our Pub ||
  | picture1     ||
  | Abe One      ||
  | Aville, AL   ||
  | For          ||
  | Amount       ||
  | Pay          ||
  And with options:
  | groceries |  

Scenario: a member card is charged
  Given members have:
  | uid  | selling   | coFlags |*
  | .ZZC | groceries | tip     |
  And cryptcookie "qid" is "NEWZZB"

  When member "?" completes "card/6vm/KDJIAa1" with:
  | op  | amount | desc      |*
  | pay | 10     | groceries |
  Then we say "status": "report tx" with subs:
  | did  | otherName | amount |*
  | paid | Our Pub   | $10    |
  And these "txs":
  | xid | created | amount | payer | payee | purpose   | taking |*
  |   1 | %today  |     10 | .ZZA  | .ZZC  | groceries | 0      |
  And we show "Pay: Our Pub" with:
  | Scan a CG Card |
  And without:
  | Show My QR |
  | Undo |
  | Tip |
  | Receipt |
