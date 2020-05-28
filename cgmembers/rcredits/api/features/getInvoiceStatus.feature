Feature: Get invoice status

Setup:
  Given members:
  | uid  | fullName   | email | cc  | cc2  | floor | phone      | address     | city       | state | zip   | flags             |*
  | .ZZA | Abe One    | a@    | ccA | ccA2 |  -250 | 2345678901 |             |            | MA    |       | ok,ided,confirmed |
  | .ZZB | Bea Two    | b@    | ccB | ccB2 |  -250 |            | 123 Main St | Greenfield | MA    | 01301 | ok,ided,confirmed |
  | .ZZC | Corner Pub | c@    | ccC |      |     0 |            |             |            | MA    |       | ok,ided,confirmed |

  And relations:
  | main | agent | flags   |*
  | .ZZA | .ZZC  | autopay |
  
Scenario: member invoices another member and gets PENDING then gets invoice status
  Given member ".ZZA" with password "123" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | nonce | purpose                   |*
  | .ZZB    | 25.00  | %today      | %(%today+1m) | 1235  | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status  | payerId | amount | cgInvoiceId | error |*
  | 1235  | PENDING | .ZZB    | 25.00  | 1           | ?     |

  When member ".ZZA" with password "123" sends "get-invoice-status" requests:
  | cgInvoiceId | amount | payerId |*
  | 1           | 25.00  | .ZZB    |

  Then the response op is "get-invoice-status-response" and the status is "OK" and there are 1 responses and they are:
   | status  | payerId | amount | cgInvoiceId | error |*
   | PENDING | .ZZB    | 25.00  | 1           | ?     |
   

Scenario: member invoices another member and gets APPROVED then gets invoice status
  Given member ".ZZA" with password "123" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | nonce | purpose                   |*
  | .ZZC    | 29.00  | %today      | %(%today+1m) | 1238  | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status   | payerId | amount | cgInvoiceId | error |*
  | 1238  | APPROVED | .ZZC    | 29.00  | 1           | ?     |

  And balances:
  | uid  | balance |*
  | .ZZA | 0       |
  | .ZZB | 0       |
  | .ZZC | 0       |

  When member ".ZZA" with password "123" sends "get-invoice-status" requests:
  | cgInvoiceId | amount | payerId |*
  | 1           | 29.00  | .ZZC    |

  Then the response op is "get-invoice-status-response" and the status is "OK" and there are 1 responses and they are:
  | status   | payerId | amount | cgInvoiceId | error |*
  | APPROVED | .ZZC    | 29.00  | 1           | ?     |



Scenario: member wants to invoice another member and the invoice is auto-paid then gets invoice status
  Given  balances:
  | uid  | balance |*
  | .ZZC | 225     |

  When member ".ZZA" with password "123" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | nonce | purpose                   |*
  | .ZZC    | 29.00  | %today      | %(%today+1m) | 1238  | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status   | payerId | amount | cgInvoiceId | error |*
  | 1238  | PAID     | .ZZC    | 29.00  | 1           | ?     |

  And balances:
  | uid  | balance |*
  | .ZZA | 29      |
  | .ZZB | 0       |
  | .ZZC | 196     |

  When member ".ZZA" with password "123" sends "get-invoice-status" requests:
  | cgInvoiceId | amount | payerId |*
  | 1           | 29.00  | .ZZC    |

  Then the response op is "get-invoice-status-response" and the status is "OK" and there are 1 responses and they are:
  | status   | payerId | amount | cgInvoiceId | error |*
  | PAID     | .ZZC    | 29.00  | 1           | ?     |


Scenario: member wants to get invoice status and fails because they don't own it
  Given member ".ZZA" with password "123" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | nonce | purpose                   |*
  | .ZZC    | 27.00  | %today      | %(%today+1m) | 1239  | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status   | payerId | amount | cgInvoiceId | error |*
  | 1239  | APPROVED | .ZZC    | 27.00  | 1           | ?     |

  When member ".ZZA" with password "123" sends "get-invoice-status" requests:
  | cgInvoiceId | amount | payerId |*
  | 1           | 27.00  | .ZZC    |

  Then the response op is "get-invoice-status-response" and the status is "OK" and there are 1 responses and they are:
  | status   | payerId | amount | cgInvoiceId | error |*
  | APPROVED | .ZZC    | 27.00  | 1           | ?     |

  When member ".ZZB" with password "123" sends "get-invoice-status" requests:
  | cgInvoiceId | amount | payerId |*
  | 1           | 27.00  | .ZZC    |

  Then the response op is "get-invoice-status-response" and the status is "OK" and there are 1 responses and they are:
  | status | payerId | amount | cgInvoiceId | error            |*
  | BAD    | .ZZC    | 27.00  | ?           | no invoice found |


Scenario: member wants to get invoice status for a nonexistent invoice
  When member ".ZZA" with password "123" sends "get-invoice-status" requests:
  | payerId | amount | cgInvoiceId |*
  | .ZZC    | 27.00  | 2           |

  Then the response op is "get-invoice-status-response" and the status is "OK" and there are 1 responses and they are:
  | status | payerId | cgInvoiceId | error            |*
  | BAD    | .ZZC    | 2           | no invoice found |
