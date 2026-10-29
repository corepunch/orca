(Node2D Name="InboxContent" class="chat-background"
  (Pivot Name="Inbox" Title="Messenger" ExtendBodyBehindHeader=true class="toolbar-page chat-ink"
    (PivotItem Name="FilterAll" Header="All"
      (Node2D
        (Node2D
          (ItemsStack Name="ConversationListAll" Overflow="hidden scroll" JustifyContent="Start" ItemNamePrefix="Room" ItemsSource=(bind "DataContext/AllRooms/Items") ItemTemplate="Chat/Templates/RoomRow"
            (StackView Name="InboxErrorAll" Visible=(bind "DataContext/AllRooms/ShowError") Spacing=12 Padding="0 8" JustifyContent="Start"
              (TextBlock Text="Couldn't load conversations." FontSize=22 class="chat-muted" IgnoreHitTest=true)
              (Button Name="RetryInboxAll" Click="Rooms.Retry" Height=48 FontSize=15 class="chat-action chat-link" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" "Retry"))
            (StackView Name="InboxEmptyAll" Visible=(bind "DataContext/AllRooms/ShowEmpty") Spacing=8 Padding="0 8" JustifyContent="Start"
              (TextBlock FontSize=22 class="chat-muted" IgnoreHitTest=true Text=(if (bind "DataContext/Matching/Params/Search") "no conversations found" "you're all caught up"))
              (TextBlock FontSize=14 WordWrap=true class="chat-dim" IgnoreHitTest=true Text=(if (bind "DataContext/Matching/Params/Search") "Try another name or message." "Start a new chat, or swipe to another list."))))
          (ActivityIndicator Name="InboxLoadingAll" Visible=(bind "DataContext/AllRooms/ShowLoading") class="chat-muted"))
        (Node2D Name="SearchBar" SafeAreaEdge="Top" class="frosted bar-bottom" MarginLeft=-20 MarginRight=-20 Padding="20 0 20 12"
          (Grid Columns="1fr 44px" Height=48
            (Input Name="Search" PlaceholderText="search" Height=44 FontSize=15 Padding="12 0" BorderWidth=2 class="chat-input chat-field" Text=(bind "DataContext/Matching/Params/Search" Mode=TwoWay))
            (Button Name="ClearSearch" Click="ClearSearch" Width=44 Height=48 BorderWidth=0 class="chat-action chat-field"
              (VectorView Source="Chat/assets/icons/x.svg" Width=20 Height=20 MarginLeft="auto" MarginRight="auto" MarginTop="auto" MarginBottom="auto" class="chat-field-ink" IgnoreHitTest=true))))))
    (PivotItem Name="FilterUnread" Header="Unread"
      (Node2D
        (ItemsStack Name="ConversationListUnread" Overflow="hidden scroll" JustifyContent="Start" ItemNamePrefix="Room" ItemsSource=(bind "DataContext/UnreadRooms/Items") ItemTemplate="Chat/Templates/RoomRow"
          (StackView Name="InboxErrorUnread" Visible=(bind "DataContext/UnreadRooms/ShowError") Spacing=12 Padding="0 8" JustifyContent="Start"
            (TextBlock Text="Couldn't load conversations." FontSize=22 class="chat-muted" IgnoreHitTest=true)
            (Button Name="RetryInboxUnread" Click="Rooms.Retry" Height=48 FontSize=15 class="chat-action chat-link" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" "Retry"))
          (StackView Name="InboxEmptyUnread" Visible=(bind "DataContext/UnreadRooms/ShowEmpty") Spacing=8 Padding="0 8" JustifyContent="Start"
            (TextBlock FontSize=22 class="chat-muted" IgnoreHitTest=true Text=(if (bind "DataContext/Matching/Params/Search") "no conversations found" "you're all caught up"))
            (TextBlock FontSize=14 WordWrap=true class="chat-dim" IgnoreHitTest=true Text=(if (bind "DataContext/Matching/Params/Search") "Try another name or message." "Start a new chat, or swipe to another list."))))
        (ActivityIndicator Name="InboxLoadingUnread" Visible=(bind "DataContext/UnreadRooms/ShowLoading") class="chat-muted")))
    (PivotItem Name="FilterPinned" Header="Pinned"
      (Node2D
        (ItemsStack Name="ConversationListPinned" Overflow="hidden scroll" JustifyContent="Start" ItemNamePrefix="Room" ItemsSource=(bind "DataContext/PinnedRooms/Items") ItemTemplate="Chat/Templates/RoomRow"
          (StackView Name="InboxErrorPinned" Visible=(bind "DataContext/PinnedRooms/ShowError") Spacing=12 Padding="0 8" JustifyContent="Start"
            (TextBlock Text="Couldn't load conversations." FontSize=22 class="chat-muted" IgnoreHitTest=true)
            (Button Name="RetryInboxPinned" Click="Rooms.Retry" Height=48 FontSize=15 class="chat-action chat-link" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" "Retry"))
          (StackView Name="InboxEmptyPinned" Visible=(bind "DataContext/PinnedRooms/ShowEmpty") Spacing=8 Padding="0 8" JustifyContent="Start"
            (TextBlock FontSize=22 class="chat-muted" IgnoreHitTest=true Text=(if (bind "DataContext/Matching/Params/Search") "no conversations found" "you're all caught up"))
            (TextBlock FontSize=14 WordWrap=true class="chat-dim" IgnoreHitTest=true Text=(if (bind "DataContext/Matching/Params/Search") "Try another name or message." "Start a new chat, or swipe to another list."))))
        (ActivityIndicator Name="InboxLoadingPinned" Visible=(bind "DataContext/PinnedRooms/ShowLoading") class="chat-muted"))))
  (TextBlock Name="InboxStatus" Height=24 SafeAreaEdge="Bottom" Visible=false FontSize=11 TextHorizontalAlignment="Center" class="frosted chat-dim" IgnoreHitTest=true))
