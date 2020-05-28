Feature: Sign up users

Setup:
  Given members:
  | uid  | fullName   | email | cc  | cc2  | floor | phone      | address     | city       | state | zip  |*
  | .ZZA | Abe One    | a@    | ccA | ccA2 |  -250 | 2345678901 |             |            | MA    |       |
  | .ZZB | Bea Two    | b@    | ccB | ccB2 |  -250 |            | 123 Main St | Greenfield | MA    | 01301 |
  | .ZZC | Corner Pub | c@    | ccC |      |     0 |            |             |            | MA    |       |

Scenario: member wants to sign up another member and succeeds
  Given member ".ZZA" with password "123" sends "sign-up-users" requests:
  | fullName   | email | phone      | address     | city       | state | zipCode | nonce |*
  | Dee Four   | d@    | 1234567890 | 125 Main St | Greenfield | MA    | 01301   | 378   |
  
  Then the response op is "sign-up-users-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status | cgId |*
  | 378   | OK     | .AAA |

Scenario: member wants to sign up several members all of which succeed
  Given member ".ZZA" with password "123" sends "sign-up-users" requests:
  | fullName   | email | phone      | address     | city       | state | zipCode | nonce |*
  | Dee Four   | d@    | 1234567890 | 125 Main St | Greenfield | MA    | 01301   | 378   |
  | Eve Five   | e@    | 4137777777 | 123 Main St | Greenfield | MA    | 01301   | 379   |
  | Far Co     |       |            |             |            | MA    |         | 380   |

  Then the response op is "sign-up-users-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status | cgId | error |*
  | 378   | OK     | .AAA | ?     |
  | 379   | OK     | .AAC | ?     |
  | 380   | OK     | .AAD | ?     |
