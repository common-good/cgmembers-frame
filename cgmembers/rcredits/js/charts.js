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
var ctty = getv.ctty;
var site = getv.site;
var region = getv.theregion;
var gridlines = 6;
var pData, vs;
var chartName = getv.chart;
var chart = null; // the chart object
var period;

var chartAreaW = '50%'; // leave room for yAxis labels and legend
var chartW = 960; // was 480
var chartH = 300;
if (getv.selectable) {chartW = 600; chartH = 400;}

var helpline = $('#help-line');
var chData = $('#chart-data').html();
chData = chData.substr(4, chData.length - 7); // trim off the comment markers
chData = JSON.parse(chData);

chartHelp = {
  'success':'success-metrics',
  'funds':'dollar-pool',
  'growth':'growth',
  'banking':'bank-transfers',
  'volume':'transaction-volume',
  'velocity':'circulation-velocity'
};

$('#ctty').change(function () {recall(chart, $(this).val());});
$('#chart').change(function () {setChart($(this).val());});
$('#atom').change(function () {setPeriod();});

google.load('visualization', '1.0', {'packages':['corechart']});
google.setOnLoadCallback(window.setPeriod);  


function successChart() {
  var data = myRows('successData', 'Success, Active, Gifts / 5, Payees * 50, Basket, Invites');
  var colors = 'blue green silver red yellow orange';
  doChart('Success metric: ' + vs.success, data, colors);
}

function growthChart() {
  var data = myRows('growthData', 'Companies, Members, Joining, Active'); // Conx, Local Conx
  var colors = 'BLUE GREEN silver red'; // yellow orange
  doChart('Accounts: ' + vs.accts, data, colors);
}

function fundsChart() {
  var columns = `CG Credits, Dollar Pool, Savings, Top 3${vs.topPct}, Bottom 3${vs.topPct}, Credit Limits, Bals < 0`;
  var data = myRows('fundsData', columns);
  var colors = '#00CC00 BLUE yellow red red magenta orange';
  doChart('Dollar Pool Total: ' + vs.funds, data, colors);
}

function velocityChart() {
  var data = myRows('velocityData', 'Inter-cmty, Local, Bank Transfers');
  var colors = 'YELLOW #00CC00 blue';
// hAxis title:'What fraction of Common Good Credits turn over monthly', 
  var options = {
    vAxis: {format:'percent', viewWindow:{min:0, max:1.5}},
  };
  doChart('Circulation Velocity: ' + vs.velocity, data, colors, options);
}

function bankingChart() {
  var columns = 'FROM Bank, TO Bank';
  if (ctty != 0) columns += ', Exports';
  columns += (ctty == 0 ? ', Inter-cmty Trade' : ', Imports');
  var colors = 'GREEN ORANGE lightblue yellow';
  var data = myRows('bankingData', columns, ctty == 0 ? 3 : 0);
//  if (ctty == 0) options['series'].splice(3, 1); // remove "Exports", which actually means intra
  doChart('Monthly Bank Transfers: ' + vs.usd, data, colors);
}

function volumeChart() {
  var data = myRows('volumeData', 'p2p, p2b, b2b, b2p');
  var colors = 'orange green blue red';
  doChart('Monthly Transactions: ' + vs.txs, data, colors);
}

function recall(chart, ctty) {
  var myUrl = site.search('cgmembers') > 0 ? 'http://localhost/cgmembers-frame/cgmembers/rcredits/misc' : 'https://cg4.us';
  window.location = myUrl + '/chart.php?selectable=1&chart=' + chartName + '&ctty=' + ctty + '&site=' + site + '&region=' + region;
};

/**
 * Draw an area chart with the given data and options.
 */
function doChart(title, data, colors, params = {}) {
  var series, i, c;
  
  series = [];
  colors = colors.split(' ');
  for (var i in colors) {
    c = colors[i];
    series[i] = {
      areaOpacity: c == c.toUpperCase() ? 1 : 0, // uppercase color means solid area (otherwise just a line)
      color: c,
    };
  }

  var options = { // set default parameters
    title:title,
    series: series,
    width: chartW, height:chartH,
    hAxis: {
//      viewWindow: {min:new Date(vs.dt1 * 1000)}, 
      format: period == 'y' ? 'yyyy' : ('mq'.includes(period) ? 'MMM`yy' : (period == 'w' ? 'MMMdd' : 'Edd')), // ` not ' (chrome bug)
      gridlines: {count:gridlines}, 
      title: '', 
      titleTextStyle: {color:'darkgray'}
    },
    vAxis: {format:'short'},
    chartArea: {width:chartAreaW},
    legend: {position:'right'}
  };
    
  chart = new google.visualization.AreaChart(document.getElementById('onechart'));
  chart.draw(data, Object.assign({}, options, params));
}

/**
 * Add a row to the table.
 * @param string dataName: name of the dataset, embedded in the html
 * @param string columns: comma-space-delimited list of series labels
 * @param int remove: index of column to remove, if any
 */
function myRows(dataName, columns, remove) {
  var i;
  var data = new google.visualization.DataTable();
  data.addColumn('date', 'Date');
  columns = columns.split(', ');
  for (i in columns) data.addColumn('number', columns[i]);

  var dataSet = JSON.parse(JSON.stringify(pData[dataName])); // get a COPY of the original dataset (multiply by 1000 only once)
  for (i in dataSet) {
    dataSet[i][0] = new Date(dataSet[i][0] * 1000);
    if (remove) dataSet[i].splice(remove, 1); 
    data.addRow(dataSet[i]);    
  }
  return data;
};

/**
 * Change the help link.
 */
function chgHelp(from, to) {
  from = from == '' ? 'CHARTHELP' : chartHelp[from];
  to = chartHelp[to];
  helpline.html(helpline.html().replace(from, to));
}

/**
 * Display a different chart.
 */
function setChart(newChart) {
  var context = $('#chart');
  $('option', context).removeClass();
  $(':selected', context).addClass('selected');

  chgHelp(chartName, newChart);
  chartName = newChart;
  if (chart != null) chart.clearChart();
  var fn = window[chartName + 'Chart'];
  fn();
}

/**
 * Switch to the dataset for a different period (granularity).
 */
function setPeriod() {
  period = $('#atom').val();
  pData = chData[period];
  vs = pData.vs;
  setChart(chartName); // refresh current chart with new data
}
