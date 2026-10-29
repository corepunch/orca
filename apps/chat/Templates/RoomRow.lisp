(Button Name="Room" Height=76 class="conversation" Click="OpenRoom" StateClasses=(bind "DataContext/State")
  (Grid Columns="56px 1fr 44px" Spacing=14 Padding="0 10" IgnoreHitTest=true
    (Grid Name="Avatar" Width=56 Height=56 class="avatar" StateClasses=(bind "DataContext/Tone") IgnoreHitTest=true
      (TextBlock Text=(bind "DataContext/Initials") FontSize=18 class="avatar-initials" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" IgnoreHitTest=true)
      (ImageView Name="AvatarPhoto" Visible=(bind "DataContext/HasAvatar") Src=(bind "DataContext/AvatarPath") ForegroundColor="#ffffff" Stretch="UniformToFill" IgnoreHitTest=true))
    (StackView JustifyContent="Center" Spacing=2 IgnoreHitTest=true
      (TextBlock Name="RoomTitle" Text=(bind "DataContext/Title") FontSize=22 TextOverflow="Ellipsis" TextWrapping="NoWrap" class="chat-ink" IgnoreHitTest=true)
      (TextBlock Name="RoomPreview" Text=(bind "DataContext/Preview") FontSize=14 TextOverflow="Ellipsis" TextWrapping="NoWrap" class="room-preview" IgnoreHitTest=true))
    (StackView AlignItems="End" Spacing=8 Padding="0 6" JustifyContent="Start" IgnoreHitTest=true
      (TextBlock Name="RoomTime" Text=(bind "DataContext/Time") FontSize=12 class="room-preview" IgnoreHitTest=true)
      (TextBlock Name="UnreadBadge" Visible=(bind "DataContext/HasUnread") Text=(bind "DataContext/UnreadText") Width=20 Height=20 FontSize=11 FontWeight="Bold" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" class="chat-badge" IgnoreHitTest=true)
      (VectorView Name="PinIcon" Visible=(bind "DataContext/ShowPin") Source="Chat/assets/icons/pin.svg" Width=14 Height=14 MarginLeft="auto" MarginRight="auto" MarginTop="auto" MarginBottom="auto" class="chat-dim" IgnoreHitTest=true))))
