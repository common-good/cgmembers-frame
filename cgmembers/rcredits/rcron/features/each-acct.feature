Feature: Each Account
AS a Common Good region
I WANT to examine each account for problems
SO we can be confident about the integrity of every account.

Setup:
  Given members:
  | uid  | fullName | flags   |*
  | .ZZA | Abe One  | ok,confirmed      |
  | .ZZB | Bea Two  | ok,confirmed      |
  | .ZZC | Cor Pub  | ok,confirmed,co   |
  
Scenario: A user becomes an international criminal
  Given these "ofac":
  | nm      | co |*
  | Bea Two | 0  |
  And variable "ofac_updated" is %yesterday
  When cron runs "eachAcct"
  Then we tell admin "Name flagged as criminal by latest OFAC data" with ray:
  | fullName | qid    |*
  | Bea Two  | NEWZZB |
