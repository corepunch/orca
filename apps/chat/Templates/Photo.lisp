(Grid Name="PhotoFrame" Width=260 Height=(bind "DataContext/FrameHeight") class="chat-soft"
  (Grid Name="PhotoContent"
    (VectorView Name="PhotoLoading" Visible=(bind "DataContext/Pending") Source="Chat/assets/icons/refresh-cw.svg" Width=32 Height=32 MarginLeft="auto" MarginRight="auto" MarginTop="auto" MarginBottom="auto" class="chat-muted" IgnoreHitTest=true)
    (ImageView Name="Photo" Visible=(bind "DataContext/HasPhoto") Src=(bind "DataContext/PhotoPath") ForegroundColor="#ffffff" Stretch="UniformToFill" IgnoreHitTest=true))
  (Button Name="OpenPhoto" Click="OpenPhoto" IgnoreHitTest=(bind "DataContext/Failed") BackgroundColor="#00000000" BorderWidth=0)
  (Button Name="RetryPhoto" Visible=(bind "DataContext/Failed") Click="Photos.Retry" Height=44 FontSize=12 MarginTop="auto" MarginBottom="auto" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" BorderWidth=0 class="chat-action chat-link" "photo unavailable · retry")
  (TextBlock Name="PhotoTime" Visible=(bind "DataContext/ShowTime") Text=(bind "DataContext/Timestamp") FontSize=11 Padding="4 2" MarginLeft="auto" MarginTop="auto" MarginRight=6 MarginBottom=6 class="photo-time" IgnoreHitTest=true))
