Feature: Changes
AS a community administrator
I WANT to review the significant changes to an rCredits account
SO I can provide informed customer service

AS an overall administrator
I WANT to review the significant changes to an rCredits account
SO I can request changes to software, that will enhance the experience of rCredits members

Setup:
  Given members:
  | uid  | fullName | address | city | state | flags    | minimum | achMin | saveWeekly | crumbs | activated | bankAccount |*
  | .ZZA | Abe One  | 1 A St. | Aton | MA    | ok,ided  |     100 |     10 |          0 |   0.02 | %today-2y |             |
  | .ZZB | Bea Two  | 2 B St. | Bton | MA    | ok,debt  |     200 |     20 |          0 |   0.05 | %today-2y |             |
  | .ZZC | Cor Pub  | 3 C St. | Cton | CA    | ok,co    |     300 |     30 |          0 |   0.05 | %today-2y |             |
  | .ZZD | Dee Four | 4 D St. | Dton | DE    | ok,admin |     400 |     40 |          0 |   0.05 | %today-2y |             |
  And these "admins":
  | uid  | vKeyE     | can                 |*
  | .ZZD |           | seeAccts,seeChanges |
  
Scenario: A member changes some settings
  Given member ".ZZA" completes form "settings/preferences" with values:
  | roundup | statements | secretBal |*
  |       0 | paper      |         0 |
  And member ".ZZA" has done step "fund"
  And member ".ZZA" completes form "settings/fund" with values:
  | connect | routingNumber | bankAccount | bankAccount2 | cashout | refills | target | achMin | saveWeekly |*
  |       1 |     211870281 |         123 |          123 |       0 |       1 |    100 |     11 |          0 |
  When member ".ZZD" visits page "sadmin/changes/NEWZZA"
  Then we show "Account Changes for Abe One" with:
  | %mdY | flags       | ok ided       | ok ided              | abeone     |
  And with:
  | %mdY | flags       | ok ided       | ok ided paper        | abeone     |
  And with:
  | %mdY | flags       | ok ided paper | ok ided refill paper | abeone     |
  And with:
  | %mdY | achMin      |            10 |                   11 | abeone     |
  And with:
  | %mdY | bankAccount |               | (secret)             | abeone     |
# sometimes this happens out of order (dunno why)

#  | %mdY | bankAccount |                      | USkk211870281123            | abeone     | (this will work with member "D:1" when decry('V') works)
#  | %mdY | flags   | member ok bona | member ok bona weekly debt |
