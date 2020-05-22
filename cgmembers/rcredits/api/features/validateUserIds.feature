Feature: Validate User Ids

Setup:
  Given members:
  | uid  | fullName   | email | cc  | cc2  | floor | flags             | helper |*
  | .ZZA | Abe One    | a@    | ccA | ccA2 |  -250 | ok,confirmed,debt | 0      |
  | .ZZB | Bea Two    | b@    | ccB | ccB2 |  -250 | ok,confirmed,debt | 0      |
  | .ZZC | Corner Pub | c@    | ccC |      |     0 | ok,co,confirmed   | .ZZA   |
  | .ZZD | Dee Four   | d@    | ccD | ccD2 |     0 | ok,confirmed      | 0      |
  | .ZZE | Eve Five   | e@    | ccE | ccE2 |  -250 | ok,secret,roundup,debt | .ZZD   |
  | .ZZF | Far Co     | f@    | ccF |      |     0 | ok,co,confirmed   | 0      |

Scenario: user wants to validate another customer account
  Given user ".ZZA" with password "123" asks API whether these users are valid:
  | cgId | fullName   | email | phone      | address     | city       | state | zipCode |*
  | .ZZB | Bea TWo    | b@    |            | 123 Main St | Greenfield | MA    | 01301   |
  | .ZZD | Dee Four   | d4@   | 1234567890 | 124 Main St | Greenfield | MA    | 01301   |
  | .ZZG | Gary Seven | g@    |            | 125 Main St | Greenfield | MA    | 01301   |
  
  Then the response op is "validate-user-ids-response" and the status is "OK" and there are 3 responses and they are:
  | status | cgId | errors      |*
  | OK     | .ZZB |             |
  | OK     | .ZZD |             |
  | BAD    |      | bad account |
