function doGet(e) { 
  Logger.log( JSON.stringify(e) );
  var result = 'Ok';
  if (e.parameter == 'undefined') {
    result = 'No Parameters';
  }
  else {
    var sheet_id = ''; 	// Spreadsheet ID from Google sheet Url
    var sheet = SpreadsheetApp.openById(sheet_id).getActiveSheet();
    var newRow = sheet.getLastRow() + 1;						
    var rowData = [];
    var Curr_Date = new Date();
    rowData[0] = Curr_Date; // Date in column A
    var Curr_Time = Utilities.formatDate(Curr_Date, "Asia/Bangkok", 'HH:mm:ss');
    rowData[1] = Curr_Time; // Time in column B
    for (var param in e.parameter) {
      Logger.log('In for loop, param=' + param);
      var value = stripQuotes(e.parameter[param]);
      Logger.log(param + ':' + e.parameter[param]);
      switch (param) {
        case 'temp':
          rowData[2] = value; // XY-MD02 Temperature in column C
          result = 'XY-MD02 Temperature Written on column C'; 
          break;
        case 'humi':
          rowData[3] = value; // XY-MD02 Humidity in column D
          result = 'XY-MD02 Humidity Written on column D'; 
          break;
          
          case 'temp2':
          rowData[4] = value; // DHT11 Temperature in column E
          result = 'DHT11 Temperature Written on column E'; 
          break;
        case 'humi2':
          rowData[5] = value; // DHT11 Humidity in column F
          result = 'DHT11 Humidity Written on column F'; 
          break;
        case 'soil1':
          rowData[6] = value; // Soil Moisture Sensor in column G
          result = 'Soil Moisture Written on column G'; 
          break;
      }
    }
    Logger.log(JSON.stringify(rowData));
    var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
    newRange.setValues([rowData]);
  }
  return ContentService.createTextOutput(result);
}
function stripQuotes( value ) {
  return value.replace(/^["']|['"]$/g, "");
}