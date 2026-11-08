param([Parameter(Mandatory=$true)][string]$Directory)
$ErrorActionPreference='Stop'
Add-Type -AssemblyName System.Windows.Forms.DataVisualization
Add-Type -AssemblyName System.Drawing
$dgaRows=Import-Csv -LiteralPath (Join-Path $Directory 'game-curves.csv')
$dgaNames=@($dgaRows | Select-Object -ExpandProperty scenario -Unique)
$dgaColors=@('#2574a9','#d17a16','#19835e')
for($dgaPage=0;$dgaPage -lt [math]::Ceiling($dgaNames.Count/10);$dgaPage++){
    $dgaChart=New-Object System.Windows.Forms.DataVisualization.Charting.Chart
    $dgaPageRows=[math]::Ceiling([math]::Min(10,$dgaNames.Count-$dgaPage*10)/2)
    $dgaChart.Width=1800;$dgaChart.Height=180+400*$dgaPageRows;$dgaChart.BackColor=[Drawing.Color]::White
    $dgaTitle=$dgaChart.Titles.Add('Visible stadium fullness through the game');$dgaTitle.Font=New-Object Drawing.Font('Arial',19,[Drawing.FontStyle]::Bold)
    $dgaTitle.Position.Auto=$false;$dgaTitle.Position.X=0;$dgaTitle.Position.Y=0;$dgaTitle.Position.Width=100;$dgaTitle.Position.Height=60/$dgaChart.Height*100
    $dgaLegend=New-Object System.Windows.Forms.DataVisualization.Charting.Legend('venues');$dgaLegend.Docking='Top';$dgaLegend.Font=New-Object Drawing.Font('Arial',11);$dgaLegend.Position.Auto=$false;$dgaLegend.Position.X=10;$dgaLegend.Position.Y=62/$dgaChart.Height*100;$dgaLegend.Position.Width=80;$dgaLegend.Position.Height=35/$dgaChart.Height*100;$dgaChart.Legends.Add($dgaLegend)
    for($dgaSlot=0;$dgaSlot -lt 10;$dgaSlot++){
        $dgaIndex=$dgaPage*10+$dgaSlot;if($dgaIndex -ge $dgaNames.Count){break}
        $dgaName=$dgaNames[$dgaIndex];$dgaArea=New-Object System.Windows.Forms.DataVisualization.Charting.ChartArea('a'+$dgaSlot)
        $dgaPanelRow=[math]::Floor($dgaSlot/2)
        $dgaArea.Position.Auto=$false;$dgaArea.Position.X=2+($dgaSlot%2)*49;$dgaArea.Position.Y=(145+400*$dgaPanelRow)/$dgaChart.Height*100;$dgaArea.Position.Width=47;$dgaArea.Position.Height=360/$dgaChart.Height*100
        $dgaArea.AxisX.Minimum=0;$dgaArea.AxisX.Maximum=48;$dgaArea.AxisX.Interval=12;$dgaArea.AxisX.Title='Elapsed game minutes (12-minute quarters)'
        $dgaArea.AxisY.Minimum=0;$dgaArea.AxisY.Maximum=100;$dgaArea.AxisY.Interval=25;$dgaArea.AxisY.Title='Fullness (% of capacity)'
        $dgaArea.AxisX.LabelStyle.Font=New-Object Drawing.Font('Arial',10);$dgaArea.AxisY.LabelStyle.Font=New-Object Drawing.Font('Arial',10)
        $dgaArea.AxisX.TitleFont=New-Object Drawing.Font('Arial',10);$dgaArea.AxisY.TitleFont=New-Object Drawing.Font('Arial',10)
        $dgaArea.AxisX.MajorGrid.LineColor=[Drawing.Color]::LightGray;$dgaArea.AxisY.MajorGrid.LineColor=[Drawing.Color]::LightGray
        $dgaChart.ChartAreas.Add($dgaArea)
        $dgaHeading=$dgaChart.Titles.Add($dgaName);$dgaHeading.Font=New-Object Drawing.Font('Arial',12,[Drawing.FontStyle]::Bold)
        $dgaHeading.Position.Auto=$false;$dgaHeading.Position.X=$dgaArea.Position.X;$dgaHeading.Position.Y=(110+400*$dgaPanelRow)/$dgaChart.Height*100;$dgaHeading.Position.Width=47;$dgaHeading.Position.Height=30/$dgaChart.Height*100
        $dgaColorIndex=0
        foreach($dgaSeats in @('6060','30000','61888')){
            $dgaSeries=New-Object System.Windows.Forms.DataVisualization.Charting.Series ('s'+$dgaSlot+'-'+$dgaSeats)
            $dgaSeries.ChartArea=$dgaArea.Name;$dgaSeries.ChartType='Line';$dgaSeries.BorderWidth=3;$dgaSeries.Color=[Drawing.ColorTranslator]::FromHtml($dgaColors[$dgaColorIndex]);$dgaSeries.Legend='venues';$dgaSeries.LegendText=([int]$dgaSeats).ToString('N0')+' authored positions';$dgaSeries.IsVisibleInLegend=$dgaSlot -eq 0
            foreach($dgaRow in $dgaRows){if($dgaRow.scenario -eq $dgaName -and $dgaRow.seats -eq $dgaSeats){[void]$dgaSeries.Points.AddXY(([double]$dgaRow.elapsed_reference_seconds/60),[double]$dgaRow.fullness_pct)}}
            $dgaChart.Series.Add($dgaSeries);$dgaColorIndex++
        }
        $dgaFloor=New-Object System.Windows.Forms.DataVisualization.Charting.StripLine
        $dgaFloor.IntervalOffset=50;$dgaFloor.BorderColor=[Drawing.Color]::Gray;$dgaFloor.BorderDashStyle='Dash';$dgaFloor.BorderWidth=1;$dgaArea.AxisY.StripLines.Add($dgaFloor)
    }
    $dgaChart.SaveImage((Join-Path $Directory ('fullness-'+($dgaPage+1)+'.png')),'Png');$dgaChart.Dispose()
}
Write-Output 'Rendered fullness sheets with the Windows charting library.'
