Feature: Make payments

Setup:
  Given members:
  | uid  | fullName   | email | emailCode | flags             |*
  | .ZZA | Abe One    | a@    | 11111     | ok,ided,confirmed |
  | .ZZB | Bea Two    | b@    | 22222     | ok,ided,confirmed |
  | .ZZC | Corner Pub | c@    | 33333     | ok,co,ided,confirmed |

  And these "u_relations":
  | main | agent | flags   |*
  | .ZZC | .ZZA  | autopay |
  
Scenario: member wants to pay another member and succeeds
  Given balances:
  | uid  | balance |*
  | .ZZC | 240     |
  | .ZZB | 20      |
  
  When member ".ZZC" with password "33333" sends "make-payments" requests:
  | payeeId | amount | billingDate | nonce | purpose                   |*
  | .ZZB    | 25.00  | %today      | 1235  | 15% of credits for 13 KWH |

  Then the response op is "make-payments-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status  | payeeId | amount | error |*
  | 1235  | OK      | .ZZB    | 25.00  | ?     |

  And balances:
  | uid  | balance |*
  | .ZZC | 215     |
  | .ZZB | 45      |


Scenario: member wants to pay another member but does not have enough money
  Given member ".ZZC" with password "33333" sends "make-payments" requests:
  | payeeId | amount | billingDate | nonce | purpose                   |*
  | .ZZA    | 29.00  | %today      | 1238  | 15% of credits for 13 KWH |

  Then the response op is "make-payments-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status   | payeeId | amount | error |*
  | 1238  | NSF      | .ZZA    | 29.00  | ?     |

  And balances:
  | uid  | balance |*
  | .ZZC | 0       |
  | .ZZB | 0       |
  | .ZZA | 0       |
  
Scenario: member wants to pay another member and fails
  Given member ".ZZC" with password "33333" sends "make-payments" requests:
  | payeeId | amount | billingDate | nonce | purpose                   |*
  | .ZZD    | 27.00  | %today      | 1239  | 15% of credits for 13 KWH |

  Then the response op is "make-payments-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status  | payeeId | amount | error                   |*
  | 1239  | BAD     | .ZZD    | 27.00  | payee account not found |


Scenario: a member pays another member twice for the same thing and it was done the first time
  Given  balances:
  | uid  | balance |*
  | .ZZC | 240     |
  | .ZZA | 225     |

  When member ".ZZC" with password "33333" sends "make-payments" requests:
  | payeeId | amount | billingDate | nonce | purpose                   |*
  | .ZZA    | 29.00  | %today      | 1238  | 15% of credits for 13 KWH |

  And member ".ZZC" with password "33333" sends "make-payments" requests:
  | payeeId | amount | billingDate | nonce | purpose                   |*
  | .ZZA    | 29.00  | %today      | 1238  | 15% of credits for 13 KWH |

  Then the response op is "make-payments-response" and the status is "OK" and there are 1 responses and they are:
  | nonce | status    | payeeId | amount | error  |*
  | 1238  | DUPLICATE | .ZZA    | 29.00  | ?      |

  And balances:
  | uid  | balance |*
  | .ZZC | 211     |
  | .ZZB | 0       |
  | .ZZA | 254     |
