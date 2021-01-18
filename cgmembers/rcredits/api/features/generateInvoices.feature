Feature: Sign up users

Setup:
  Given members:
  | uid  | fullName   | email | floor | emailCode | phone      | flags                |*
  | .ZZA | Abe One    | a@    |  -250 | 11111     | 2345678901 | ok,ided,confirmed    |
  | .ZZB | Bea Two    | b@    |  -250 | 22222     |            | ok,ided,confirmed    |
  | .ZZC | Corner Pub | c@    |     0 | 33333     |            | ok,co,ided,confirmed |

  And relations:
  | main | other | flags   |*
  | .ZZC | .ZZA  | autopay |
  
Scenario: member wants to invoice another member and succeeds
  Given member ".ZZC" with password "33333" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | nonce | purpose                   |*
  | .ZZB    | 25.00  | %today      | %(%today+1m) | 1235  | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status  | payerId | amount | cgInvoiceId | error |*
  | 1235  | PENDING | .ZZB    | 25.00  | 1           | ?     |

Scenario: member wants to invoice another member and the invoice is approved
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
  
Scenario: member wants to invoice another member and the invoice is auto-paid
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

Scenario: member wants to invoice another member and fails
  Given member ".ZZC" with password "33333" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | nonce | purpose                   |*
  | .ZZD    | 27.00  | %today      | %(%today+1m) | 1239  | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status  | payerId | amount | cgInvoiceId | error           |*
  | 1239  | BAD     | .ZZD    | 27.00  | ?           | Payer not found |


Scenario: a member invoices another member twice for the same thing and it was paid the first time
  Given  balances:
  | uid  | balance |*
  | .ZZA | 225     |

  When member ".ZZC" with password "33333" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | nonce | purpose                   |*
  | .ZZA    | 29.00  | %today      | %(%today+1m) | 1238  | 85% of credits for 13 KWH |

  And member ".ZZC" with password "33333" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | nonce | purpose                   |*
  | .ZZA    | 29.00  | %today      | %(%today+1m) | 1238  | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status         | payerId | amount | cgInvoiceId | error |*
  | 1238  | PAID-DUPLICATE | .ZZA    | 29.00  | 1           | ?     |

  And balances:
  | uid  | balance |*
  | .ZZA | 196     |
  | .ZZB | 0       |
  | .ZZC | 29      |

Scenario: a member invoices another member twice for the same thing and it was approved the first time
  When member ".ZZC" with password "33333" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | nonce | purpose                   |*
  | .ZZA    | 29.00  | %today      | %(%today+1m) | 1238  | 85% of credits for 13 KWH |

  And member ".ZZC" with password "33333" sends "generate-invoices" requests:
  | payerId | amount | billingDate | dueDate      | nonce | purpose                   |*
  | .ZZA    | 29.00  | %today      | %(%today+1m) | 1238  | 85% of credits for 13 KWH |

  Then the response op is "generate-invoices-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status             | payerId | amount | cgInvoiceId | error |*
  | 1238  | APPROVED-DUPLICATE | .ZZA    | 29.00  | 1           | ?     |

  And balances:
  | uid  | balance |*
  | .ZZA | 0       |
  | .ZZB | 0       |
  | .ZZC | 0       |
