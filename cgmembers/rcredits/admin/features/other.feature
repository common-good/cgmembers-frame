Feature: Activate
AS a regional administrator
I WANT to do miscellaneous stuff
SO everything works smoothly

Setup:
  Given members:
  | uid  | fullName | address | city | state | zip | email | flags                   | minimum | federalId |*
  | .ZZA | Abe One  | 1 A St. | Aton | MA    | 01000      | a@    | ok,admin         |     100 | 111111111 |
  | .ZZB | Bea Two  | 2 B St. | Bton | MA    | 02000      | b@    | ok               |     200 | 222222222 |
  | .ZZD | Dee Four | 4 D St. | Dton | MA    | 04000      | d@    | member,confirmed |     400 | 444444444 |

# This scene assures that field changes made to users in migrations get made also to x_users
Scenario: Admin deletes an account
  When member "B:A" completes form "PHP" with values:
  | code |*
  | r\deleteAccount('NEWZZD'); |
  Then these "x_users":
  | uid  | fullName | deleted |*
  | .ZZD | Dee Four | %now    |
