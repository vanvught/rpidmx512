function formatDateTime(date) {
    const year = date.getFullYear();
    const mon = ('0' + (date.getMonth() + 1)).slice(-2);
    const day = ('0' + date.getDate()).slice(-2);
    const hour = ('0' + date.getHours()).slice(-2);
    const min = ('0' + date.getMinutes()).slice(-2);
    const sec = ('0' + date.getSeconds()).slice(-2);
    const offset = date.getTimezoneOffset();
    const offsetHour = ('0' + Math.floor(Math.abs(offset) / 60)).slice(-2);
    const offsetMin = ('0' + Math.abs(offset) % 60).slice(-2);
    const sign = offset <= 0 ? '+' : '-';
    
    if ((offsetHour == 0) && (offsetMin == 0)) {
      return `${year}-${mon}-${day}T${hour}:${min}:${sec}Z`;  
    } 
    
    return `${year}-${mon}-${day}T${hour}:${min}:${sec}${sign}${offsetHour}:${offsetMin}`;   
}