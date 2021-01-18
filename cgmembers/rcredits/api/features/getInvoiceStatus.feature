Feature: Get invoice status

Setup:
  Given members:
  | uid  | fullName   | email | emailCode | flags             |*
  | .ZZA | Abe One    | a@    | 11111     | ok,ided,confirmed |
  | .ZZB | Bea Two    | b@    | 22222     | ok,ided,confirmed |
  | .ZZC | Corner Pub | c@    | 33333     | ok,co,ided,confirmed |
  | .ZZD | Dee Four   | d@    | 44444     | ok,co,ided,confirmed |

  And relations:
  | main | agent | flags   |*
  | .ZZC | .ZZA  | autopay |
  
Scenario: member invoices another member and gets PENDING then gets invoice status
  Given member ".ZZC" with password "33333" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | nonce | purpose                   |*
  | .ZZB    | 25.00  | %today      | %(%today+1m) | 1235  | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status  | payerId | amount | cgInvoiceId | error |*
  | 1235  | PENDING | .ZZB    | 25.00  | 1           | ?     |

  When member ".ZZC" with password "33333" sends "get-invoice-status" requests:
  | cgInvoiceId | amount | payerId |*
  | 1           | 25.00  | .ZZB    |

  Then the response op is "get-invoice-status-response" and the status is "OK" and there are 1 responses and they are:
   | status  | payerId | amount | cgInvoiceId | error |*
   | PENDING | .ZZB    | 25.00  | 1           | ?     |
   

Scenario: member invoices another member and gets APPROVED then gets invoice status
  Given member ".ZZC" with password "33333" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | nonce | purpose                   |*
  | .ZZA    | 29.00  | %today      | %(%today+1m) | 1238  | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status   | payerId | amount | cgInvoiceId | error |*
  | 1238  | APPROVED | .ZZA    | 29.00  | 1           | ?     |

  And balances:
  | uid  | balance |*
  | .ZZC | 0       |
  | .ZZB | 0       |
  | .ZZA | 0       |

  When member ".ZZC" with password "33333" sends "get-invoice-status" requests:
  | cgInvoiceId | amount | payerId |*
  | 1           | 29.00  | .ZZA    |

  Then the response op is "get-invoice-status-response" and the status is "OK" and there are 1 responses and they are:
  | status   | payerId | amount | cgInvoiceId | error |*
  | APPROVED | .ZZA    | 29.00  | 1           | ?     |



Scenario: member wants to invoice another member and the invoice is auto-paid then gets invoice status
  Given  balances:
  | uid  | balance |*
  | .ZZA | 225     |

  When member ".ZZC" with password "33333" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | nonce | purpose                   |*
  | .ZZA    | 29.00  | %today      | %(%today+1m) | 1238  | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status   | payerId | amount | cgInvoiceId | error |*
  | 1238  | PAID     | .ZZA    | 29.00  | 1           | ?     |

  And balances:
  | uid  | balance |*
  | .ZZC | 29      |
  | .ZZB | 0       |
  | .ZZA | 196     |

  When member ".ZZC" with password "33333" sends "get-invoice-status" requests:
  | cgInvoiceId | amount | payerId |*
  | 1           | 29.00  | .ZZA    |

  Then the response op is "get-invoice-status-response" and the status is "OK" and there are 1 responses and they are:
  | status   | payerId | amount | cgInvoiceId | error |*
  | PAID     | .ZZA    | 29.00  | 1           | ?     |


Scenario: member wants to get invoice status and fails because they don't own it
  Given member ".ZZC" with password "33333" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | nonce | purpose                   |*
  | .ZZA    | 27.00  | %today      | %(%today+1m) | 1239  | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status   | payerId | amount | cgInvoiceId | error |*
  | 1239  | APPROVED | .ZZA    | 27.00  | 1           | ?     |

  When member ".ZZC" with password "33333" sends "get-invoice-status" requests:
  | cgInvoiceId | amount | payerId |*
  | 1           | 27.00  | .ZZA    |

  Then the response op is "get-invoice-status-response" and the status is "OK" and there are 1 responses and they are:
  | status   | payerId | amount | cgInvoiceId | error |*
  | APPROVED | .ZZA    | 27.00  | 1           | ?     |

  When member ".ZZD" with password "44444" sends "get-invoice-status" requests:
  | cgInvoiceId | amount | payerId |*
  | 1           | 27.00  | .ZZA    |

  Then the response op is "get-invoice-status-response" and the status is "OK" and there are 1 responses and they are:
  | status | payerId | amount | cgInvoiceId | error            |*
  | BAD    | .ZZA    | 27.00  | ?           | no invoice found |


Scenario: member wants to get invoice status for a nonexistent invoice
  When member ".ZZC" with password "33333" sends "get-invoice-status" requests:
  | payerId | amount | cgInvoiceId |*
  | .ZZA    | 27.00  | 2           |

  Then the response op is "get-invoice-status-response" and the status is "OK" and there are 1 responses and they are:
  | status | payerId | cgInvoiceId | error            |*
  | BAD    | .ZZA    | 2           | no invoice found |
