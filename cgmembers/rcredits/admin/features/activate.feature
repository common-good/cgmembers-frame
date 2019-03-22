Feature: Activate
AS a regional administrator
I WANT to activate an rCredits account
SO the new member can participate

Setup:
  Given members:
  | uid  | fullName | address | city | state | zip | email | flags                   | minimum | federalId |*
  | .ZZA | Abe One  | 1 A St. | Aton | MA    | 01000      | a@    | ok,admin         |     100 | 111111111 |
  | .ZZB | Bea Two  | 2 B St. | Bton | MA    | 02000      | b@    | ok               |     200 | 222222222 |
  | .ZZD | Dee Four | 4 D St. | Dton | MA    | 04000      | d@    | member,confirmed |     400 | 444444444 |
  And relations:
  | main | agent | num | permission |*
  | .ZZD | .ZZA  |   1 | manage     |
# relationship is here only so we can identify which account admin is managing

Scenario: Admin activates an account
  Given member ".ZZD" has no photo ID recorded
  When member "D:A" completes form "summary" with values:
  | mediaConx | rTrader | helper  | federalId  | adminable        | tickle |*
  |         1 |       1 | Bea Two | %R_ON_FILE | member,confirmed |        |
  Then members:
  | uid  | flags               | helper |*
  | .ZZD | member,confirmed,ok |   .ZZB |

