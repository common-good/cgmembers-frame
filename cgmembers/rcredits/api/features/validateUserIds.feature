Feature: Validate User Ids

Setup:
  Given members:
  | uid  | fullName   | email | emailCode | floor | phone      | address     | city       | state | zip   | flags |*
  | .ZZA | Abe One    | a@    | 11111     |  -250 | 2345678901 |             |            | MA    |       | ok    |
  | .ZZB | Bea Two    | b@    | 22222     |  -250 |            | 123 Main St | Greenfield | MA    | 01301 | ok    |
  | .ZZC | Corner Pub | c@    | 33333     |     0 |            |             |            | MA    |       | ok,co |
  | .ZZD | Dee Four   | d@    | 44444     |     0 | 1234567890 | 123 Main St | Greenfield | MA    | 01301 | ok    |
  | .ZZE | Eve Five   | e@    | 55555     |  -250 | 4137777777 | 123 Main St | Greenfield | MA    | 01301 | ok    |
  | .ZZF | Far Co     | f@    | 66666     |     0 |            |             |            | MA    |       | ok,co |

Scenario: member wants to validate another member account and succeeds
  Given member ".ZZC" with password "33333" sends "validate-user-ids" requests:
  | cgId | fullName   | email | phone      | address     | city       | state | zipCode |*
  | .ZZA | Abe One    | a@    |            | 12 Main St  | Greenfield | MA    | 01301   |
  
  Then the response op is "validate-user-ids-response" and the status is "OK" and there are 1 responses and they are:
  | status | cgId | error |*
  | OK     | .ZZA | ?     |

Scenario: user wants to validate another customer account and fails
  Given member ".ZZC" with password "33333" sends "validate-user-ids" requests:
  | cgId | fullName   | email | phone                | address       | city       | state | zipCode |*
  | .ZZA | Abe One    |       | 7777777777           | 25 Federal St | Greenfield | MA    | 01301   |
  
  Then the response op is "validate-user-ids-response" and the status is "OK" and there are 1 responses and they are:
  | status | cgId | error                                                          |*
  | BAD    | .ZZA | That does not appear to be your correct Common Good member ID. |

Scenario: user wants to validate several customer accounts some of which succeed
  Given member ".ZZC" with password "33333" sends "validate-user-ids" requests:
  | cgId | fullName   | email | phone      | address     | city       | state | zipCode |*
  | .ZZB | Bea TWo    | b@    |            | 123 Main St | Greenfield | MA    | 01301   |
  | .ZZD | Dee Four   | d4@   | 1234567890 | 124 Main St | Greenfield | MA    | 01301   |
  | .ZZE | John R     | john@ | 4137777777 | 37 Nowhere  | Northfield | MA    | 99999   |
  | .ZZG | Gary Seven | g@    |            | 125 Main St | Greenfield | MA    | 01301   |
  
  Then the response op is "validate-user-ids-response" and the status is "OK" and there are 4 responses and they are:
  | status | cgId | error                                                          |*
  | OK     | .ZZB | ?                                                              |
  | OK     | .ZZD | ?                                                              |
  | BAD    | .ZZE | That does not appear to be your correct Common Good member ID. |
  | BAD    | .ZZG | Common Good Account not found                                  |

Scenario: user wants to validate another account with wrong password
  Given member ".ZZC" with password "456" sends "validate-user-ids" requests:
  | cgId | fullName   | email | phone      | address     | city       | state | zipCode |*
  | .ZZB | Bea TWo    | b@    |            | 123 Main St | Greenfield | MA    | 01301   |
  | .ZZD | Dee Four   | d4@   | 1234567890 | 124 Main St | Greenfield | MA    | 01301   |
  | .ZZE | John R     | john@ | 4137777777 | 37 Nowhere  | Northfield | MA    | 99999   |
  | .ZZG | Gary Seven | g@    |            | 125 Main St | Greenfield | MA    | 01301   |
  
  Then the response op is "validate-user-ids-response" and the status is "BAD" and the error is: "Company id NEWZZC not found or wrong password"
 