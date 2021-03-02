Feature: Daily
AS a member
I WANT appropriate actions to be taken with respect to my account
SO things will be as they should be.

Setup:
  Given members:
  | uid  | fullName | address | city       | state | zip   | flags                 |*
  | .ZZA | Abe One  | 1 A St. | Greenfield | MA    | 01301 | ok,confirmed,refill   |
  | .ZZB | Bea Two  | 2 B St. | Greenfield | MA    | 01301 | ok,confirmed,cashoutW |
  | .ZZC | Cor Pub  | 3 C St. | Ctown      | CT    | 03000 | ok,co,confirmed       |
  And these "people":
  | pid | fullName | address | city       | state | zip   |*
  | 1   | Dot Four | 4 D St. | Greenfield | MA    | 01301 |
  
Scenario: Geoposition gets calculated
  When cron runs "periodic"
  Then members have:
  | uid  | latitude  | longitude |*
  | .ZZA | %GFLD_LAT | %GFLD_LON |
  And these "people":
  | pid  | latitude  | longitude |*
  | 1    | %GFLD_LAT | %GFLD_LON |

Scenario: A member has a big day
  Given transactions:
  | xid | created    | amount | payer | payee | purpose |*
  |   1 | %yesterday |    500 | .ZZB  | .ZZC  | food    |
  |   2 | %yesterday |    600 | .ZZA  | .ZZB  | stuff   |
  |   3 | %today     |   2000 | .ZZB  | .ZZA  | stuff   |
  When cron runs "biggies"
  Then we tell admin "Big Transfers on %dmy" with ray:
  | Bea Two | 1100 |

#Scenario: A trial company runs out of time
#  Given members:
#  | uid       | .AAA           |**
#  | fullName  | Coco Co        |
#  | email     | a@             |
#  | flags     | confirmed co depends ok |
#  | activate  | %now-15d       |
#  When cron runs "everyDay"
#  Then we message "trial co end" to member ".AAA"
#  And members have:
#  | uid  | flags        | task |*
#  | .AAA | confirmed co | co2  |
