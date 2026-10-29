(Node2D Name="DetailsContent"
  (StackView Name="DetailsBody" Spacing=14 Overflow="hidden scroll" JustifyContent="Start"
    (Grid Name="RoomAvatar" Width=72 Height=72 StateClasses=(bind "DataContext/Room/Tone") IgnoreHitTest=true
      (TextBlock Text=(bind "DataContext/Room/Initials") FontSize=22 class="avatar-initials" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" IgnoreHitTest=true)
      (ImageView Visible=(bind "DataContext/Room/HasAvatar") Src=(bind "DataContext/Room/AvatarPath") ForegroundColor="#ffffff" Stretch="UniformToFill" IgnoreHitTest=true))
    (TextBlock Name="RoomKind" Text=(bind "DataContext/Room/KindText") Width=100 Height=24 FontSize=11 FontWeight="Bold" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" class="chat-soft chat-accent-text" IgnoreHitTest=true)
    (TextBlock Name="RoomAbout" Text=(bind "DataContext/Room/About") FontSize=14 WordWrap=true class="chat-muted" IgnoreHitTest=true)
    (TextBlock Name="DetailsStatus" Visible=false FontSize=14 WordWrap=true class="chat-muted")
    (StackView Name="RenameSection" Visible=(bind "DataContext/Room/CanRename") Spacing=14 JustifyContent="Start"
      (TextBlock Text="conversation name" FontSize=18 MarginTop=8 class="chat-accent-text" IgnoreHitTest=true)
      (Input Name="RenameRoomName" PlaceholderText="Conversation name" Height=44 FontSize=15 Padding="12 0" BorderWidth=2 Text=(bind "DataContext/Rename/Title" Mode=TwoWay) class="chat-input chat-field")
      (Button Name="RenameRoom" Click="Rename.Submit" Height=44 BorderWidth=2 FontSize=15 FontWeight="Bold" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" class="chat-action chat-outline" "Save name"))
    (TextBlock Text="people" FontSize=18 MarginTop=8 class="chat-accent-text" IgnoreHitTest=true)
    (ItemsStack Name="Members" Spacing=14 JustifyContent="Start" ItemNamePrefix="Member" ItemsSource=(bind "DataContext/Members/Items") ItemTemplate="Chat/Templates/MemberRow")
    (Grid Name="ContentLoading" Visible=(bind "DataContext/Members/ShowLoading") Height=80 IgnoreHitTest=true
      (ActivityIndicator class="chat-muted"))
    (StackView Name="MembersFailed" Visible=(bind "DataContext/Members/ShowError") Spacing=16 JustifyContent="Start"
      (TextBlock Text=(bind "DataContext/Members/ErrorMessage") FontSize=14 WordWrap=true class="chat-error" IgnoreHitTest=true)
      (Button Name="RetryContent" Click="Members.Retry" Height=48 FontSize=15 TextHorizontalAlignment="Left" TextVerticalAlignment="Center" BorderWidth=0 class="chat-action chat-link" "Retry"))
    (StackView Name="InviteSection" Visible=(bind "DataContext/Room/CanInvite") Spacing=14 JustifyContent="Start"
      (TextBlock Text="invite people" FontSize=18 MarginTop=8 class="chat-accent-text" IgnoreHitTest=true)
      (TextBlock Name="Invitation" Text=(bind "DataContext/Room/InviteText") FontSize=14 WordWrap=true class="chat-muted")
      (Input Name="InvitationCode" Visible=(bind "DataContext/Room/HasInvite") Text=(bind "DataContext/Room/InviteCode") PlaceholderText="Invitation code" Height=44 FontSize=15 Padding="12 0" BorderWidth=2 class="chat-input chat-field")
      (Button Name="CreateInvite" Click="CreateInvite" Height=44 BorderWidth=2 FontSize=15 FontWeight="Bold" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" class="chat-action chat-outline" "Create invitation")
      (Button Name="RevokeInvite" Click="RevokeInvite" Height=48 FontSize=15 TextHorizontalAlignment="Left" TextVerticalAlignment="Center" BorderWidth=0 class="chat-action chat-error" "Revoke invitation"))
    (StackView Name="LeaveSection" Visible=(bind "DataContext/Room/CanLeave") Spacing=8 JustifyContent="Start"
      (Button Name="LeaveRoom" Click="AskLeave" Text=(bind "DataContext/Room/LeaveText") Height=48 FontSize=15 TextHorizontalAlignment="Left" TextVerticalAlignment="Center" BorderWidth=0 class="chat-action chat-error")
      (StackView Name="LeaveRoomConfirmation" Visible=(bind "DataContext/Room/ConfirmLeave") Spacing=8 JustifyContent="Start"
        (TextBlock Text="Leave this conversation? You will need an invitation to return to a private conversation." FontSize=14 WordWrap=true class="chat-muted" IgnoreHitTest=true)
        (Button Name="ConfirmLeaveRoom" Click="Leave" Height=48 FontSize=15 TextHorizontalAlignment="Left" TextVerticalAlignment="Center" BorderWidth=0 class="chat-action chat-error" "Leave conversation")
        (Button Name="CancelLeaveRoom" Click="CancelLeave" Height=44 BorderWidth=2 FontSize=15 FontWeight="Bold" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" class="chat-action chat-outline" "Cancel"))))
  (ApplicationBar Name="AppBar" SafeAreaEdge="Bottom" ContentInsetAdjustment="Always" class="frosted bar-top chat-ink" PaddingLeft=(bind "Screen.SafeAreaLeft") PaddingRight=(bind "Screen.SafeAreaRight")
    (ApplicationBarIconButton Name="PinConversation" IconUri="Chat/assets/icons/pin.svg" Text=(bind "DataContext/Room/PinText") Click="TogglePin")))
