(Node2D Name="ThreadContent"
  (StackView Name="Messages" Spacing=0 PaddingBottom=8 Overflow="hidden scroll" ScrollAnchor="Bottom" JustifyContent="Start"
    (TextBlock Name="TimelineDate" Visible=(bind "DataContext/Messages/HasLoaded") Text=(bind "DataContext/Thread/Date") FontSize=12 TextHorizontalAlignment="Center" MarginTop=4 MarginBottom=6 class="chat-dim" IgnoreHitTest=true)
    (Button Name="EarlierMessages" Visible=(bind "DataContext/Messages/ShowHistory") Click="LoadHistory" IgnoreHitTest=(bind "DataContext/Messages/IsLoadingMore") Height=44 BackgroundColor="#00000000" BorderWidth=0
      (TextBlock Name="HistoryText" Text=(bind "DataContext/Thread/HistoryText") FontSize=12 TextHorizontalAlignment="Center" TextVerticalAlignment="Center" class="chat-muted" IgnoreHitTest=true))
    (StackView Name="EmptyState" Visible=(bind "DataContext/Messages/ShowEmpty") Spacing=8 Padding="0 8" JustifyContent="Start"
      (TextBlock Name="EmptyTimeline" Text=(bind "DataContext/Thread/EmptyTitle") FontSize=22 TextWrapping="Wrap" class="chat-muted" IgnoreHitTest=true)
      (TextBlock Text=(bind "DataContext/Thread/EmptyHint") FontSize=14 TextWrapping="Wrap" class="chat-dim" IgnoreHitTest=true))
    (ItemsStack Name="MessageItems" Spacing=0 JustifyContent="Start" ItemNamePrefix="MessageEntry" ItemsSource=(bind "DataContext/Messages/Items") ItemTemplate="Chat/Templates/Message"))
  (ActivityIndicator Name="MessagesLoading" Visible=(bind "DataContext/Messages/ShowLoading") class="chat-muted"))
