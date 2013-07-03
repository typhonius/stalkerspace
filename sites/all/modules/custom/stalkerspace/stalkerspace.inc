<?php

class StalkerspaceEntityFieldQuery extends EntityFieldQuery {
  public function __construct() {
    $this->entityCondition('entity_type', 'node')
      ->propertyCondition('type', 'confession')
      ->propertyCondition('status', 1);
      //->propertyOrderBy(‘created’, ‘DESC’);
  }
}

class StalkerspaceUserEntityFieldQuery extends EntityFieldQuery {
  public function __construct() {
    $this->entityCondition('entity_type', 'user')
      ->propertyCondition('status', 1);
  }
}