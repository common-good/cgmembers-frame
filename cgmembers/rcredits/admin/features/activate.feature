Feature: Activate
AS a regional administrator
I WANT to activate (or deactivate) a Common Good account
SO the new member can participate (or stop participating)

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
  And we message "approved" to member ".ZZD" with subs:
  | youName  | inviterName | specifics | otherName |*
  | Dee Four | Bea Two     |         ? |           |

Scenario: Admin deactivates an account
  Then members:
  | uid  | flags          |*
  | .ZZB | ok,member,ided |
  # (tests add the ided bit by default when creating an active account)
  When member "B:A" completes form "summary" with values:
  | rTrader | federalId  | adminable | tickle |*
  |         | %R_ON_FILE | member    |        |
  Then members:
  | uid  | flags       |*
  | .ZZB | member,ided |