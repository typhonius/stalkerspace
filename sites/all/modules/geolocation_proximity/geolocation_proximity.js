Drupal.behaviors.geolocation_proximity = {
  attach: function(context) {
    var $ = jQuery;
    $('.geolocation-address-geocode').click(function() {
      var location = $('.geolocation-address :input').val();
      geocoder = new google.maps.Geocoder();
      geocoder.geocode( { 'address': location }, function(results, status) {
        if (status == google.maps.GeocoderStatus.OK) {
          map.setCenter(results[0].geometry.location);
        }
      });
    });
  }
}
