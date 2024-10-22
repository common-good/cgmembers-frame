<?php
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;
use \Stripe as str;

/**
 * Handle a Stripe function, using the Stripe PHP APK
 *
 * Test cards:
 *    4242424242424242 good
 *    4000000000000002 generic decline
 *    4000000000009995 insufficient funds
 *    4000000000000341 decline after attaching
 */
function stripe($op, $args = []) {
  require_once DRUPAL_ROOT . '/../vendor/autoload.php';
  str\Stripe::setApiKey(STRIPE_SECRET);
  return $op($args);
}

/**
 * Create a Stripe Customer
 * @return the Stripe customer object
 */
function cust($args) {
  extract(just('fullName email phone zip country', $args, NULL));

  $info = ray('name email phone address', $fullName, $email, $phone, ray('postal_code country', $zip, 'US')); // not $country, which has to be an integer
  if (test()) return obj('id', u\code());
  
  try {
    return str\Customer::create($info); // we need a customer record whether recurring or not
  } catch (Exception $e) { return $e->getMessage(); }
}

/**
 * Get a Stripe SetupIntent
 * @return the setupIntent object OR an error message
 */
function setup($args) {
  extract(just('stripeCid description', $args, NULL));

  $customer = $stripeCid;
  $payment_method_types = ray('card'); // You can specify different payment methods (especially us_bank_account)
  $usage = 'off_session';
  $use_stripe_sdk = TRUE;
  $info = compact(ray('customer description payment_method_types usage use_stripe_sdk'));
  if (test()) return obj('id client_secret', u\code(), u\code());
  
  try {
    return str\SetupIntent::create($info);
  } catch (Exception $e) { return $e->getMessage(); }
}

/**
 * Create a PaymentIntent to charge the saved payment method
 * @return: the PaymentIntent object OR an error message
 */
function charge($args) {
  extract(just('stripeId shouldStart to amount', $args)); 
  if (test()) return obj('id', u\code());
  
  $setupIntent = str\SetupIntent::retrieve($stripeId);
  $paymentMethodId = $setupIntent->payment_method;
  $customerId = $setupIntent->customer;
  $idempotency_key = "$shouldStart.$stripeId";
           
  $info = [
    'amount' => round($amount * 100), // amount in cents ($50.00)
    'currency' => 'usd',
    'customer' => $customerId, // Use the stored customer ID
    'payment_method' => $paymentMethodId, // Use the stored payment method ID
    'off_session' => true, // Off-session indicates the customer is not interacting
    'confirm' => true, // Confirm the payment immediately
  ];
  
  try {
    return str\PaymentIntent::create($info, compact('idempotency_key'));
  } catch (Exception $e) { return $e->getMessage(); }
}
