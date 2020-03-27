Feature: Daily
AS a member
I WANT appropriate actions to be taken with respect to my account
SO things will be as they should be.

Setup:
  Given members:
  | uid  | fullName | address | city       | state | zip   | flags                 |*
  | .ZZA | Abe One  | 1 A St. | Greenfield | MA    | 01301 | ok,confirmed,refill   |
  | .ZZB | Bea Two  | 2 B St. | Greenfield | MA    | 01301 | ok,confirmed,cashoutW |

Scenario: Geoposition gets calculated
  When cron runs "periodic"
  Then members have:
  | uid  | latitude  | longitude |*
  | .ZZA | %GFLD_LAT | %GFLD_LON |

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
