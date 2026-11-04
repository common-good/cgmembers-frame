Feature: Database transactions
AS a Developer
I WANT a transaction to be undone when its job is rolled back or fails
SO we never save half of a multi-step job.

Scenario: A transaction ends normally
  When a transaction sets variable "zzTx" and ends: "normally"
  Then variable "zzTx" is set

Scenario: A transaction is rolled back
  When a transaction sets variable "zzTx" and ends: "rolls back"
  Then variable "zzTx" is not set

Scenario: A job fails inside a function holding the transaction
  When a transaction sets variable "zzTx" and ends: "fails"
  Then variable "zzTx" is not set

Scenario: A caller rolls back a failed job and later jobs still commit
  When a transaction sets variable "zzTx" and ends: "fails, caller rolls back"
  Then variable "zzTx" is not set
  
  When a transaction sets variable "zzTx2" and ends: "normally"
  Then variable "zzTx2" is set
