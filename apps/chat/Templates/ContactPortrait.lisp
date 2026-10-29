(Button class="chat-action" BorderWidth="0" BackgroundColor="#00000000"
  (Grid Name="ContactAvatar" BackgroundColor=(bind "DataContext/Tone") IgnoreHitTest=true
    (TextBlock Text=(bind "DataContext/Initials") ForegroundColor=(bind "DataContext/Ink")
      FontSize=24 TextHorizontalAlignment="Center" TextVerticalAlignment="Center" IgnoreHitTest=true)))
