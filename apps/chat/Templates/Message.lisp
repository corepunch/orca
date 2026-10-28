(StackView Name="MessageEntry" Spacing=0 JustifyContent="Start"
  (TextBlock Name="TimelineDate" Visible=(bind "DataContext/ShowDate") Text=(bind "DataContext/Date") FontSize=12
    ForegroundColor=(bind "DataContext/Dim") TextHorizontalAlignment="Center" MarginTop=4 MarginBottom=6
    IgnoreHitTest=true)
  (Button Name="EarlierMessages" Visible=(bind "DataContext/ShowHistory") IgnoreHitTest=(bind "DataContext/HistoryDisabled")
    Height=44 Background="#00000000" BorderWidth=0
    (TextBlock Text=(bind "DataContext/HistoryText") FontSize=12 ForegroundColor=(bind "DataContext/Muted")
      TextHorizontalAlignment="Center" TextVerticalAlignment="Center" IgnoreHitTest=true))
  (StackView Name="EmptyState" Visible=(bind "DataContext/Empty") Spacing=8 Padding="0 8"
    (TextBlock Name="EmptyTimeline" Text=(bind "DataContext/EmptyTitle") FontSize=22
      ForegroundColor=(bind "DataContext/Muted") TextWrapping="Wrap" IgnoreHitTest=true)
    (TextBlock Text=(bind "DataContext/EmptyHint") FontSize=14 ForegroundColor=(bind "DataContext/Dim")
      TextWrapping="Wrap" IgnoreHitTest=true))
  (StackView Name="MessageRow" Visible=(bind "DataContext/HasMessage") Spacing=0 JustifyContent="Start"
    AlignItems=(bind "DataContext/Alignment") MarginTop=(bind "DataContext/Gap")
    PaddingLeft=(bind "DataContext/LeftInset") PaddingRight=(bind "DataContext/RightInset")
    (VectorView Name="IncomingTail" Source="Chat/assets/bubble-tail-up.svg"
      Visible=(bind "DataContext/IncomingTail") Width=14 Height=12 ForegroundColor=(bind "DataContext/Tone")
      MarginLeft=16 MarginRight="auto" IgnoreHitTest=true)
    (Grid Name="MessageHolder"
      (StackView Name="MessageBubble" BackgroundColor=(bind "DataContext/Tone")
        Width=(bind "DataContext/BubbleWidth") PaddingLeft=(bind "DataContext/Inset")
        PaddingRight=(bind "DataContext/Inset") PaddingTop=(bind "DataContext/VerticalInset")
        PaddingBottom=(bind "DataContext/VerticalInset") Spacing=(bind "DataContext/SectionGap")
        JustifyContent="Start"
        (StackView Name="SenderSection" Visible=(bind "DataContext/ShowSender")
          PaddingLeft=(bind "DataContext/PhotoInset") PaddingRight=(bind "DataContext/PhotoInset")
          PaddingTop=(bind "DataContext/PhotoVerticalInset") PaddingBottom=(bind "DataContext/PhotoVerticalInset")
          JustifyContent="Start"
          (TextBlock Text=(bind "DataContext/Sender") FontSize=13 FontWeight="Bold"
            ForegroundColor=(bind "DataContext/Accent") TextWrapping="Wrap" IgnoreHitTest=true))
        (StackView Name="PhotoSection" Visible=(bind "DataContext/HasPhotos") Spacing=0 JustifyContent="Start"
          (StackView Name="Photos" Spacing=0 JustifyContent="Start"))
        (StackView Name="Caption" Visible=(bind "DataContext/ShowCaption") Spacing=2 JustifyContent="Start"
          PaddingLeft=(bind "DataContext/PhotoInset") PaddingRight=(bind "DataContext/PhotoInset")
          PaddingTop=(bind "DataContext/PhotoVerticalInset") PaddingBottom=(bind "DataContext/PhotoVerticalInset")
          (TextBlock Name="MessageBody" Visible=(bind "DataContext/HasBody") Text=(bind "DataContext/Body")
            FontSize=16 ForegroundColor=(bind "DataContext/Ink") TextWrapping="Wrap" IgnoreHitTest=true)
          (TextBlock Name="MessageTime" Text=(bind "DataContext/Timestamp") FontSize=11
            ForegroundColor=(bind "DataContext/Meta") TextHorizontalAlignment="Right" IgnoreHitTest=true)))
      (Button Name="MessageActions" Visible=(bind "DataContext/CanAct") Background="#00000000" BorderWidth=0))
    (VectorView Name="OutgoingTail" Source="Chat/assets/bubble-tail-down.svg"
      Visible=(bind "DataContext/OutgoingTail") Width=14 Height=12 ForegroundColor=(bind "DataContext/Tone")
      MarginLeft="auto" MarginRight=16 IgnoreHitTest=true)))
