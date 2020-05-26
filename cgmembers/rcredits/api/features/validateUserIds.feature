Feature: Validate User Ids

Setup:
  Given members:
  | uid  | fullName   | email | cc  | cc2  | floor | phone      | address     | city       | state | zip  |*
  | .ZZA | Abe One    | a@    | ccA | ccA2 |  -250 | 2345678901 |             |            | MA    |       |
  | .ZZB | Bea Two    | b@    | ccB | ccB2 |  -250 |            | 123 Main St | Greenfield | MA    | 01301 |
  | .ZZC | Corner Pub | c@    | ccC |      |     0 |            |             |            | MA    |       |
  | .ZZD | Dee Four   | d@    | ccD | ccD2 |     0 | 1234567890 | 123 Main St | Greenfield | MA    | 01301 |
  | .ZZE | Eve Five   | e@    | ccE | ccE2 |  -250 | 4137777777 | 123 Main St | Greenfield | MA    | 01301 |
  | .ZZF | Far Co     | f@    | ccF |      |     0 |            |             |            | MA    |       |

Scenario: user wants to validate another customer account and succeeds
  Given user ".ZZA" with password "123" asks API whether these users are valid:
  | cgId | fullName   | email | phone      | address     | city       | state | zipCode |*
  | .ZZC | Corner Pub | c@    |            | 12 Main St  | Greenfield | MA    | 01301   |
  
  Then the response op is "validate-user-ids-response" and the status is "OK" and there are 1 responses and they are:
  | status | cgId | errors      |*
  | OK     | .ZZC | ?           |

Scenario: user wants to validate another customer account and fails
  Given user ".ZZA" with password "123" asks API whether these users are valid:
  | cgId | fullName   | email | phone                | address       | city       | state | zipCode |*
  | .ZZC | Corner Pub |       | 7777777777           | 25 Federal St | Greenfield | MA    | 01301   |
  
  Then the response op is "validate-user-ids-response" and the status is "OK" and there are 1 responses and they are:
  | status | cgId | errors                                                         |*
  | BAD    | .ZZC | That does not appear to be your correct Common Good member ID. |

Scenario: user wants to validate several customer accounts some of which succeed
  Given user ".ZZA" with password "123" asks API whether these users are valid:
  | cgId | fullName   | email | phone      | address     | city       | state | zipCode |*
  | .ZZB | Bea TWo    | b@    |            | 123 Main St | Greenfield | MA    | 01301   |
  | .ZZD | Dee Four   | d4@   | 1234567890 | 124 Main St | Greenfield | MA    | 01301   |
  | .ZZE | John R     | john@ | 4137777777 | 37 Nowhere  | Northfield | MA    | 99999   |
  | .ZZG | Gary Seven | g@    |            | 125 Main St | Greenfield | MA    | 01301   |
  
  Then the response op is "validate-user-ids-response" and the status is "OK" and there are 4 responses and they are:
  | status | cgId | errors      |*
  | OK     | .ZZB | ?           |
  | OK     | .ZZD | ?           |
  | BAD    | .ZZE | That does not appear to be your correct Common Good member ID. |
  | BAD    |      | xyzz |

Scenario: user wants to validate another account with wrong password
  Given user ".ZZA" with password "456" asks API whether these users are valid:
  | cgId | fullName   | email | phone      | address     | city       | state | zipCode |*
  | .ZZB | Bea TWo    | b@    |            | 123 Main St | Greenfield | MA    | 01301   |
  | .ZZD | Dee Four   | d4@   | 1234567890 | 124 Main St | Greenfield | MA    | 01301   |
  | .ZZE | John R     | john@ | 4137777777 | 37 Nowhere  | Northfield | MA    | 99999   |
  | .ZZG | Gary Seven | g@    |            | 125 Main St | Greenfield | MA    | 01301   |
  
  Then the response op is "validate-user-ids-response" and the status is "BAD" and the error is: "Incorrect password for user NEWZZA"
