(Grid Name="PhotoFrame" Width=260 Height=(bind "DataContext/Height") BackgroundColor=(bind "DataContext/Soft")
  (Grid Name="PhotoContent"
    (TextBlock Text="Loading photo…" FontSize=12 ForegroundColor=(bind "DataContext/Muted")
      TextHorizontalAlignment="Center" TextVerticalAlignment="Center" IgnoreHitTest=true))
  (TextBlock Name="PhotoTime" Visible=(bind "DataContext/ShowTime") Text=(bind "DataContext/Timestamp") FontSize=11
    ForegroundColor="#ffffff" BackgroundColor="#00000099" Padding="4 2" MarginLeft="auto" MarginTop="auto"
    MarginRight=6 MarginBottom=6 IgnoreHitTest=true))
