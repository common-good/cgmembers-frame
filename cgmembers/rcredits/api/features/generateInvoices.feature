Feature: Sign up users

Setup:
  Given members:
  | uid  | fullName   | email | cc  | cc2  | floor | phone      | address     | city       | state | zip   | flags             |*
  | .ZZA | Abe One    | a@    | ccA | ccA2 |  -250 | 2345678901 |             |            | MA    |       | ok,ided,confirmed |
  | .ZZB | Bea Two    | b@    | ccB | ccB2 |  -250 |            | 123 Main St | Greenfield | MA    | 01301 | ok,ided,confirmed |
  | .ZZC | Corner Pub | c@    | ccC |      |     0 |            |             |            | MA    |       | ok,ided,confirmed |

  And relations:
  | main | agent | flags   |*
  | .ZZA | .ZZC  | autopay |
  
Scenario: member wants to invoice another member and succeeds
  Given member ".ZZA" with password "123" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | smtInvoiceId | purpose                   |*
  | .ZZB    | 25.00  | %today      | %(%today+1m) | 1235         | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | smtInvoiceId | status  | payerId | amount | cgInvoiceId | errors |*
  | 1235         | PENDING | .ZZB    | 25.00  | 1           | ?      |

Scenario: member wants to invoice another member and the invoice is approved
  Given member ".ZZA" with password "123" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | smtInvoiceId | purpose                   |*
  | .ZZC    | 29.00  | %today      | %(%today+1m) | 1238         | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | smtInvoiceId | status   | payerId | amount | cgInvoiceId | errors |*
  | 1238         | APPROVED | .ZZC    | 29.00  | 1           | ?      |

  And balances:
  | uid  | balance |*
  | .ZZA | 0       |
  | .ZZB | 0       |
  | .ZZC | 0       |
  
Scenario: member wants to invoice another member and the invoice is auto-paid
  Given  balances:
  | uid  | balance |*
  | .ZZC | 225     |

  When member ".ZZA" with password "123" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | smtInvoiceId | purpose                   |*
  | .ZZC    | 29.00  | %today      | %(%today+1m) | 1238         | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | smtInvoiceId | status   | payerId | amount | cgInvoiceId | errors |*
  | 1238         | PAID     | .ZZC    | 29.00  | 1           | ?      |

  And balances:
  | uid  | balance |*
  | .ZZA | 29      |
  | .ZZB | 0       |
  | .ZZC | 196     |

Scenario: member wants to invoice another member and fails
  Given member ".ZZA" with password "123" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | smtInvoiceId | purpose                   |*
  | .ZZD    | 27.00  | %today      | %(%today+1m) | 1239         | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | smtInvoiceId | status  | payerId | amount | cgInvoiceId | errors          |*
  | 1239         | BAD     | .ZZD    | 27.00  | ?           | Payer not found |


Scenario: a member invoices another member twice for the same thing and it was paid the first time
  Given  balances:
  | uid  | balance |*
  | .ZZC | 225     |

  When member ".ZZA" with password "123" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | smtInvoiceId | purpose                   |*
  | .ZZC    | 29.00  | %today      | %(%today+1m) | 1238         | 85% of credits for 13 KWH |

  And member ".ZZA" with password "123" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | smtInvoiceId | purpose                   |*
  | .ZZC    | 29.00  | %today      | %(%today+1m) | 1238         | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | smtInvoiceId | status         | payerId | amount | cgInvoiceId | errors |*
  | 1238         | PAID-DUPLICATE | .ZZC    | 29.00  | 1           | ?      |

  And balances:
  | uid  | balance |*
  | .ZZA | 29      |
  | .ZZB | 0       |
  | .ZZC | 196     |


Scenario: a member invoices another member twice for the same thing and it was approved the first time
  When member ".ZZA" with password "123" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | smtInvoiceId | purpose                   |*
  | .ZZC    | 29.00  | %today      | %(%today+1m) | 1238         | 85% of credits for 13 KWH |

  And member ".ZZA" with password "123" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | smtInvoiceId | purpose                   |*
  | .ZZC    | 29.00  | %today      | %(%today+1m) | 1238         | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | smtInvoiceId | status             | payerId | amount | cgInvoiceId | errors |*
  | 1238         | APPROVED-DUPLICATE | .ZZC    | 29.00  | 1           | ?      |

  And balances:
  | uid  | balance |*
  | .ZZA | 0       |
  | .ZZB | 0       |
  | .ZZC | 0       |
