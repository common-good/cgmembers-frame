Feature: Company Information
AS a company member
I WANT to update my company information
SO I can complete my registration and/or publicize my goods and services to other rCredits members.

Setup:
  Given members:
  | uid  | fullName | flags |*
  | .ZZA | Abe One  |       |
  | .ZZC | Our Pub  | co,ok |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | manage     |
  
Scenario: A member visits the company info page
  When member "C:A" visits page "settings/company"
  Then we show "Company Settings" with:
  |~page title | %PROJECT web page |
  | Name       | Our Pub |

Scenario: A member updates company info
  When member "C:A" confirms form "settings/company" with values:
  | private | selling | website     | description   | employees | gross | staleNudge | founded |*
  |         | stuff   | example.com | we do vittles |         2 |   100 |          3 | %mdY-1y |
  Then members:
  | uid  | selling | website     | description   | employees | gross | staleNudge | founded      |*
  | .ZZC | stuff   | example.com | we do vittles |         2 |   100 |          3 | %daystart-1y |
  And we say "status": "info saved"
  
Scenario: A member gives a bad employee count
  When member "C:A" confirms form "settings/company" with values:
  | selling | website     | description   | employees | gross |*
  | stuff   | example.com | we do vittles |        -2 |   100 |
  Then we say error in field "employees": "negative amount"

Scenario: A member gives a bad gross
  When member "C:A" confirms form "settings/company" with values:
  | selling | website     | description   | employees | gross |*
  | stuff   | example.com | we do vittles |         2 |  junk |
  Then we say error in field "gross": "bad amount"
