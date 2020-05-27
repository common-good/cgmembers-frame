Feature: Make payments

Setup:
  Given members:
  | uid  | fullName   | email | cc  | cc2  | floor | phone      | address     | city       | state | zip   | flags             |*
  | .ZZA | Abe One    | a@    | ccA | ccA2 |  -250 | 2345678901 |             |            | MA    |       | ok,ided,confirmed |
  | .ZZB | Bea Two    | b@    | ccB | ccB2 |  -250 |            | 123 Main St | Greenfield | MA    | 01301 | ok,ided,confirmed |
  | .ZZC | Corner Pub | c@    | ccC |      |     0 |            |             |            | MA    |       | ok,ided,confirmed |

  And relations:
  | main | agent | flags   |*
  | .ZZA | .ZZC  | autopay |
  
Scenario: member wants to pay another member and succeeds
  Given balances:
  | uid  | balance |*
  | .ZZA | 240     |
  | .ZZB | 20      |
  
  When member ".ZZA" with password "123" sends "make-payments" requests:
  | payeeId | amount | billingDate | smtInvoiceId | purpose                   |*
  | .ZZB    | 25.00  | %today      | 1235         | 15% of credits for 13 KWH |

  Then the response op is "make-payments-response" and the status is "OK" and there are 1 responses and they are:
  | smtInvoiceId | status  | payeeId | amount | errors |*
  | 1235         | OK      | .ZZB    | 25.00  | ?      |

  And balances:
  | uid  | balance |*
  | .ZZA | 215     |
  | .ZZB | 45      |


Scenario: member wants to pay another member but does not have enough money
  Given member ".ZZA" with password "123" sends "make-payments" requests:
  | payeeId | amount | billingDate | smtInvoiceId | purpose                   |*
  | .ZZC    | 29.00  | %today      | 1238         | 15% of credits for 13 KWH |

  Then the response op is "make-payments-response" and the status is "OK" and there are 1 responses and they are:
  | smtInvoiceId | status   | payeeId | amount | errors |*
  | 1238         | NSF      | .ZZC    | 29.00  | ?      |

  And balances:
  | uid  | balance |*
  | .ZZA | 0       |
  | .ZZB | 0       |
  | .ZZC | 0       |
  
Scenario: member wants to pay another member and fails
  Given member ".ZZA" with password "123" sends "make-payments" requests:
  | payeeId | amount | billingDate | smtInvoiceId | purpose                   |*
  | .ZZD    | 27.00  | %today      | 1239         | 15% of credits for 13 KWH |

  Then the response op is "make-payments-response" and the status is "OK" and there are 1 responses and they are:
  | smtInvoiceId | status  | payeeId | amount | errors          |*
  | 1239         | BAD     | .ZZD    | 27.00  | payee account not found |


Scenario: a member pays another member twice for the same thing and it was done the first time
  Given  balances:
  | uid  | balance |*
  | .ZZA | 240     |
  | .ZZC | 225     |

  When member ".ZZA" with password "123" sends "make-payments" requests:
  | payeeId | amount | billingDate | smtInvoiceId | purpose                   |*
  | .ZZC    | 29.00  | %today      | 1238         | 15% of credits for 13 KWH |

  And member ".ZZA" with password "123" sends "make-payments" requests:
  | payeeId | amount | billingDate | smtInvoiceId | purpose                   |*
  | .ZZC    | 29.00  | %today      | 1238         | 15% of credits for 13 KWH |

  Then the response op is "make-payments-response" and the status is "OK" and there are 1 responses and they are:
  | smtInvoiceId | status    | payeeId | amount | errors |*
  | 1238         | DUPLICATE | .ZZC    | 29.00  | ?      |

  And balances:
  | uid  | balance |*
  | .ZZA | 211     |
  | .ZZB | 0       |
  | .ZZC | 254     |
