/**
 * jQuery alert extension (thanks to Anders Abel coding.abel.nu) (modified by CG)
 */
$.extend({ alert: function (title, message, callback) {
  $('<div></div>').dialog( {
    buttons: { "Ok": function () { $(this).dialog("close"); } },
    close: function (event, ui) {
      $(this).remove(); 
      if (typeof callback !== 'undefined') callback();
    },
    resizable: false,
    title: title,
    modal: true
  }).html(message);
}
});