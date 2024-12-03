Feature: Boxes
AS a member
I WANT the device I am using to be identifies and recorded
SO I can track transactions by device and click a link in an email to pay an invoice or receive an encrypted message.

Setup:
  Given members:
  | uid  | fullName | pass | email | flags           |*
  | .ZZA | Abe One  | a1   | a@    | ok,confirmed    |
  | .ZZB | Bea Two  | b2   | b@    | ok,confirmed    |
  | .ZZC | Cor Pub  |      | c@    | ok,confirmed,co |
  And cookie "box-NEWZZA" is ""
  Then count "r_boxes" is 0
  
Scenario: A member signs in for the first time on a device
  Given next random code is "devA"
  When member "?" confirms form "signin" with values:
  | qid  | pass |*
  | .ZZA | a1   |
  Then member ".ZZA" is logged in
  And cookie "box-NEWZZA" is "devA"
  And these "r_boxes":
  | id | uid  | code | boxnum | access | created |*
  | 1  | .ZZA | devA | 1      | %now   | %now    |

Scenario: A member signs in again  
  Given next random code is "other"
  And cookie "box-NEWZZA" is "devA"
  And these "r_boxes":
  | id | uid  | code | boxnum | access  | created |*
  | 3  | .ZZA | devA | 10     | %now-1d | %now-2d |
  When member "?" confirms form "signin" with values:
  | qid  | pass |*
  | .ZZA | a1   |
  Then member ".ZZA" is logged in
  And cookie "box-NEWZZA" is "devA"
  And these "r_boxes":
  | id | uid  | code | boxnum | access | created |*
  | 3  | .ZZA | devA | 10     | %now   | %now-2d |

Scenario: A member transacts and boxId gets recorded
  Given cookie "box-NEWZZA" is "devA"
  And members have:
  | uid  | balance |*
  | .ZZA | 250     |
  And these "r_boxes":
  | id | uid  | code | boxnum | access  | created |*
  | 3  | .ZZA | devA | 1      | %now-1d | %now-2d |
  When member ".ZZA" confirms form "tx/pay" with values:
  | op  | who     | amount | goods | purpose |*
  | pay | Bea Two | 100    | %FOR_GOODS     | labor   |
  Then these "txs":
  | xid | created | amount | payer | payee | purpose | boxId |*
  |   1 | %now    |    100 | .ZZA  | .ZZB  | labor   | 3     |
