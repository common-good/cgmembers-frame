Feature: A user signs up for Common Good in order to pay
AS a newbie
I WANT to open a Common Good account
SO I can pay someone

Setup:
  Given members:
  | uid  | fullName | pass | email | flags                    | zip   | floor | emailCode |*
  | .ZZA | Abe One  | a1   | a@    | member,ok,confirmed,debt | 01001 |  -100 | Aa1       |
  | .ZZB | Bea Two  | b1   | b@    | member,ok,confirmed,debt | 01001 |  -100 | Bb2       |
  | .ZZC | Our Pub  | c1   | c@    | member,ok,co,confirmed   | 01003 |     0 | Cc3       |
  And member is logged out

Scenario: A member clicks an unsubscribe link
  Given var "code" Pencrypts:
  | email |*
  | b@    |
  When someone visits "unsubscribe/code=%code&qid=NEWZZB"
  Then we show "Unsubscribe" with:
  | tell us why |
  | Reason      |
  | Comments    |
  | Unsubscribe |

Scenario: A member follows through on unsubscribing
  Given var "code" Pencrypts:
  | email |*
  | b@    |
  When someone submits "unsubscribe/code=%code&qid=NEWZZB" with:
#  | why   | comments | email | qid  |*
  | why   | comments | email | qid  |*
  | other | yuck     | b@    | .ZZB |
  Then we redirect to "empty"
  And we say "status": "thank tell|acct deleted|return welcome"
