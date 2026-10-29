(StackView Spacing=6 Padding="0 6" JustifyContent="Start"
  (Grid Columns="52px 1fr" Spacing=12
    (Grid Name="Avatar" Width=52 Height=52 StateClasses=(bind "DataContext/Tone") IgnoreHitTest=true
      (TextBlock Text=(bind "DataContext/Initials") FontSize=18 class="avatar-initials" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" IgnoreHitTest=true)
      (ImageView Name="AvatarPhoto" Visible=(bind "DataContext/HasAvatar") Src=(bind "DataContext/AvatarPath") ForegroundColor="#ffffff" Stretch="UniformToFill" IgnoreHitTest=true))
    (StackView Spacing=2 JustifyContent="Center"
      (TextBlock Name="MemberName" Text=(bind "DataContext/Title") FontSize=22 TextOverflow="Ellipsis" TextWrapping="NoWrap" class="chat-ink" IgnoreHitTest=true)
      (TextBlock Name="MemberRole" Text=(bind "DataContext/Role") FontSize=14 class="chat-dim" IgnoreHitTest=true)))
  (Button Name="Role" Visible=(bind "DataContext/CanChangeRole") Click="SetRole" Text=(bind "DataContext/RoleAction") Height=44 BorderWidth=2 FontSize=15 FontWeight="Bold" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" class="chat-action chat-outline")
  (Button Name="Remove" Visible=(bind "DataContext/CanRemove") Click="RemoveMember" Height=48 FontSize=15 TextHorizontalAlignment="Left" TextVerticalAlignment="Center" BorderWidth=0 class="chat-action chat-error" "Remove"))
