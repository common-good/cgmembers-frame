/**
 * @file
 * Display one or all Common Good data graphs
 * @param string accts
 * @param string funds
 * @param string velocity
 * @param string usd
 * @param string txs 
 * @param string topPct: a percent sign if Top 3 means top 3 percent, otherwise empty
 * A JSON-encoded data object is embedded in the page (see var ch below). Each element is a data object for one graph.
 * NOTE!: This script is used in an iframe of cg4.us/chart.php, which in turn includes this script (here for version control)
 */
var getv = parseUrlQuery($('#script-charts').attr('src').replace(/^[^\?]+\??/,''));
//alert($('#script-charts').attr('src').replace(/^[^\?]+\??/,''));
var ctty = getv['ctty'];
var chartName = getv['chart'];
var site = getv['site'];

var ch = $('#chart-data').html();
ch = ch.substr(4, ch.length - 7); // trim off the comment markers
ch = JSON.parse(ch);
var vs = ch['vs'];
var dt1 = vs['dt1'];
var period = vs['period'];
var helpline = $('#help-line');

fixChartClass($('#chart'));

var chartAreaW = '50%'; // leave room for yAxis labels and legend
var chartW = 960; // was 480
var chartH = 300;
if (getv['selectable']) {chartW = 600; chartH = 400;}
var chart; // the chart object

chartHelp = {
  'success':'success-metrics',
  'funds':'dollar-pool',
  'growth':'growth',
  'banking':'bank-transfers',
  'volume':'transaction-volume',
  'velocity':'circulation-velocity'
};

$('#ctty').change(function () {recall(chart, $(this).val());});
$('#chart').change(function () {
  fixChartClass($(this));
  chgHelp(chartName, $(this).val());
  chartName = $(this).val();
  chart.clearChart();
  var fn = window[chartName + 'Chart'];
  fn();
});
chgHelp('', chartName);

google.setOnLoadCallback(window[chartName + 'Chart']);  

google.load('visualization', '1.0', {'packages':['corechart']});

function successChart() {
  var data = new google.visualization.DataTable();
  data.addColumn('date', 'Date');
  data.addColumn('number', 'Success');
  data.addColumn('number', 'Active');
  data.addColumn('number', 'Gifts');
  data.addColumn('number', 'Payees');
  data.addColumn('number', 'Basket');
  data.addColumn('number', 'Invites');
  myRows(data, 'successData');
        
  var options = {
    title:'Success metric: ' + vs['success'],
    width:chartW, height:chartH,
    series: [
      {areaOpacity:0, color:'blue'}, // success
      {areaOpacity:0, color:'green'}, // aAccts
      {areaOpacity:0 , color:'silver'}, // gifts
      {areaOpacity:0, color:'red'}, // payees
      {areaOpacity:0, color:'yellow'}, // basket
      {areaOpacity:0, color:'orange'} // invites
    ],
//    hAxis: {viewWindow: {min:new Date(dt1 * 1000)}, format:dtFmt(), gridlines: {count:5}, title:'', titleTextStyle: {color:'darkgray'}},
    hAxis: {viewWindow: {min:new Date(dt1 * 1000)}, format:dtFmt(), title:'', titleTextStyle: {color:'darkgray'}},
    chartArea: {width:chartAreaW},
    legend: {position:'right'}
  };

  doChart(data, options);
}

function growthChart() {
  var data = new google.visualization.DataTable();
  data.addColumn('date', 'Date');
  data.addColumn('number', 'Companies');
  data.addColumn('number', 'Members');
  data.addColumn('number', 'Joining');
  data.addColumn('number', 'Active');
//  data.addColumn('number', 'Conx');
//  data.addColumn('number', 'Local Conx');
  myRows(data, 'growthData');
  
  //ch['growthData']);

//seriesType:'bars',
//series: {5:{type:'line'}}
        
  var options = {
    title:'Accounts: ' + vs['accts'],
    width:chartW, height:chartH,
    series: [
      {areaOpacity:1, color:'blue'}, // bAccts
      {areaOpacity:1, color:'green'}, // pAccts
      {areaOpacity:0 , color:'silver'}, // newbs
      {areaOpacity:0, color:'red'} // aAccts
//      {areaOpacity:0, color:'yellow'}, // conx/aAcct
//      {areaOpacity:0, color:'orange'} // conxLocal/aAcct
    ],
//    hAxis: {viewWindow: {min:new Date(dt1 * 1000)}, format:dtFmt(), gridlines: {count:5}, title:'', titleTextStyle: {color:'darkgray'}},
    hAxis: {viewWindow: {min:new Date(dt1 * 1000)}, format:dtFmt(), title:'', titleTextStyle: {color:'darkgray'}},
    chartArea: {width:chartAreaW},
    legend: {position:'right'}
  };
  doChart(data, options);
}

function fundsChart() {
  var data = new google.visualization.DataTable();
  data.addColumn('date', 'Date');
//  data.addColumn('number', 'Bals > 0');
  data.addColumn('number', 'CG Credits');
  data.addColumn('number', 'Dollar Pool');
  data.addColumn('number', 'Savings');
  data.addColumn('number', 'Top 3' + vs['topPct']);
  data.addColumn('number', 'Bottom 3' + vs['topPct']);
  data.addColumn('number', 'Credit Limits');
  data.addColumn('number', 'Bals < 0');
//  data.addRows(ch['fundsData']);
  myRows(data, 'fundsData');

  var options = {
    title:'Dollar Pool Total: ' + vs['funds'],
    width:chartW, height:chartH,
    series: [
//      {areaOpacity:0, color:'lime'}, // Bals > 0
      {areaOpacity:1, color:'#00cc00'}, // CG Credits (lighter green)
      {areaOpacity:1, color:'blue'}, // Dollar Pool
      {areaOpacity:0, color:'yellow'}, // Savings
      {areaOpacity:0, color:'red'}, // Top 3
      {areaOpacity:0, color:'red'}, // Bottom 3
      {areaOpacity:0, color:'magenta'},  // Limits
      {areaOpacity:1, color:'orange'} // Bals < 0
    ],
    hAxis: {format:dtFmt(), gridlines: {count:5}, title:'', titleTextStyle: {color:'darkgray'}},
    vAxis: {format:'short'},
    chartArea: {width:chartAreaW},
    legend: {position:'right'}
  };

  doChart(data, options);
}

function velocityChart() {
  var data = new google.visualization.DataTable();
  data.addColumn('date', 'Date');
  data.addColumn('number', 'Inter-cmty');
  data.addColumn('number', 'Local');
  data.addColumn('number', 'Dollar Exchanges');
//  data.addRows(ch['velocityData']);
  myRows(data, 'velocityData');

  var options = {
    title:'Circulation Velocity: ' + vs['velocity'],
    width:chartW, height:chartH,
    series: [
      {areaOpacity:1, color:'yellow'}, // Inter-cmty
      {areaOpacity:1, color:'#00cc00'}, // Local
      {areaOpacity:0, color:'blue'} // USD Exchanges
    ],
    hAxis: {
      format:dtFmt(),
      gridlines: {count:5},
//      title:'What fraction of Common Good Credits turn over monthly', 
      titleTextStyle: {color:'darkgray'}
    },
    vAxis: {format:'percent', viewWindow:{min:0, max:1.5}},
    isStacked:false, // doesn't work
    chartArea: {width:chartAreaW},
    legend: {position:'right'}
  };

  doChart(data, options);
}

function bankingChart() {
  var data = new google.visualization.DataTable();
  data.addColumn('date', 'Date');
  data.addColumn('number', 'FROM Bank');
  data.addColumn('number', 'TO Bank');
  if (ctty != 0) data.addColumn('number', 'Exports');
  data.addColumn('number', ctty == 0 ? 'Inter-cmty Trade' : 'Imports');

  myRows(data, 'bankingData', ctty == 0 ? 3 : 0);

  var options = {
    title:'Monthly Bank Transfers: ' + vs['usd'],
    width:chartW, height:chartH,
    series: [
      {areaOpacity:1, color:'green'},
      {areaOpacity:1, color:'orange'},
      {areaOpacity:0, color:'lightblue'},
      {areaOpacity:0, color:'yellow'}
    ],
    hAxis: {format:dtFmt(), gridlines: {count:5}, title:'', titleTextStyle: {color:'darkgray'}},
    vAxis: {format:'short'},
    chartArea: {width:chartAreaW},
    legend: {position:'right'}
  };

  if (ctty == 0) options['series'].splice(3, 1); // remove "Exports", which actually means intra

  doChart(data, options);
}

function volumeChart() {
  var data = new google.visualization.DataTable();
  data.addColumn('date', 'Date');
  data.addColumn('number', 'p2p');
  data.addColumn('number', 'p2b');
  data.addColumn('number', 'b2b');
  data.addColumn('number', 'b2p');
//  data.addRows(ch['volumeData']);
  myRows(data, 'volumeData');

  var options = {
    title:'Monthly Transactions: ' + vs['txs'],
    width:chartW, height:chartH,
    colors: ['orange', 'green', 'blue', 'red'],
    hAxis: {format:dtFmt(), gridlines: {count:5}},
//    hAxis: {format:dtFmt(), gridlines: {count:5}, title:'(logarithmic scale)', titleTextStyle: {color:'darkgray'}},
//    vAxis: {logScale:true},
    chartArea: {width:chartAreaW},
    legend: {position:'right'}
  };

  chart = new google.visualization.LineChart(document.getElementById('onechart'));
  chart.draw(data, options);
}

function recall(chart, ctty) {
  var myUrl = site == 'dev' ? 'http://localhost/cgmembers-frame/cgmembers/rcredits/misc' : 'https://cg4.us';
  window.location = myUrl + '/chart.php?selectable=1&chart=' + chart + '&ctty=' + ctty + '&site=' + site;
};

function fixChartClass(context) {
  $('option', context).removeClass();
  $(':selected', context).addClass('selected');
}

/**
 * Draw an area chart with the given data and options.
 */
function doChart(data, options) {
  chart = new google.visualization.AreaChart(document.getElementById('onechart'));
  chart.draw(data, options);
}

function dtFmt() {return period == 'y' ? 'yyyy' : (period == 'm' ? 'MMM' : (period == 'w' ? 'MM/DD' : 'E'));}

/**
 * Add a row to the table.
 * @param obj table: the gChart table object
 * @param string dataName: name of the dataset, embedded in the html
 * @param int remove: index of column to remove, if any
 */
function myRows(table, dataName, remove) {
  var dataSet = ch[dataName];
  for (i in dataSet) {
    dataSet[i][0] = new Date(dataSet[i][0] * 1000);
    if (remove) dataSet[i].splice(remove, 1); 
    table.addRow(dataSet[i]);    
  }
};

/**
 * Change the help link.
 */
function chgHelp(from, to) {
  from = from == '' ? 'CHARTHELP' : chartHelp[from];
  to = chartHelp[to];
  helpline.html(helpline.html().replace(from, to));
}
