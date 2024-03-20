Feature: Activate
AS a regional administrator
I WANT to do miscellaneous stuff
SO everything works smoothly

Setup:
  Given members:
  | uid  | fullName | address | city | state | zip | email | flags                   | minimum | federalId |*
  | .ZZB | Bea Two  | 2 B St. | Bton | MA    | 02000      | b@    | ok,admin         |     200 | 222222222 |
  | .ZZD | Dee Four | 4 D St. | Dton | MA    | 04000      | d@    | member,confirmed |     400 | 444444444 |
  And these "admins":
  | uid  | vKeyE     | can                 |*
  | .ZZB | DEV_VKEYE | seeAccts,deleteAcct |
  And member ".ZZB" scans admin card "DEV_VKEYPW"
  
# This scene assures that field changes made to users in migrations get made also to x_users
Scenario: Admin deletes an account
  When member ".ZZB" visits page "sadmin/delete-account/NEWZZD"
  Then these "x_users":
  | uid  | fullName | deleted |*
  | .ZZD | Dee Four | %now    |

Scenario: Deleted table fields are always the same as the source table fields
  Then fields of "x_company" match
  And fields of "x_photo" match
  And fields of "x_relations" match
  And fields of "x_shout" match
  And fields of "x_txs2" match
  And fields of "x_users" match
