(StackView Name="SignalCard" Direction="Horizontal" Spacing=12 BackgroundColor="$panel-background" Padding=16 AlignItems="Center"
  (ImageView Name="SignalCardIcon" Source="{Card.Icon}" Width=28 Height=28 ForegroundColor="{Card.PrimaryColor}")
  (StackView Name="SignalCardCopy" Direction="Vertical" Spacing=2
    (TextBlock Name="SignalCardTitle" Text="{Card.Title}" FontSize=14 ForegroundColor="$text-primary")
    (TextBlock Name="SignalCardBody" Text="{Card.Body}" FontSize=12 ForegroundColor="$text-muted")))
